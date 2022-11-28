#include "ptw.h"

#include "champsim.h"
#include "util.h"
#include "vmem.h"

extern VirtualMemory vmem;
extern uint8_t warmup_complete[NUM_CPUS];

PageTableWalker::PageTableWalker(string v1, uint32_t cpu, unsigned fill_level, uint32_t v2, uint32_t v3, uint32_t v4, uint32_t v5, uint32_t v6, uint32_t v7,
                                 uint32_t v8, uint32_t v9, uint32_t v10, uint32_t v11, uint32_t v12, uint32_t v13, unsigned latency, MemoryRequestConsumer* ll)
    : champsim::operable(1), MemoryRequestConsumer(fill_level), MemoryRequestProducer(ll), NAME(v1), cpu(cpu), MSHR_SIZE(v11), MAX_READ(v12),
      MAX_FILL(v13), RQ{v10, latency}, PSCL5{"PSCL5", 4, v2, v3}, // Translation from L5->L4
      PSCL4{"PSCL4", 3, v4, v5},                                  // Translation from L5->L3
      PSCL3{"PSCL3", 2, v6, v7},                                  // Translation from L5->L2
      PSCL2{"PSCL2", 1, v8, v9},                                  // Translation from L5->L1
      CR3_addr(vmem.get_pte_pa(cpu, 0, vmem.pt_levels).first)
{
}

void PageTableWalker::handle_read()
{
  int reads_this_cycle = MAX_READ;

  while (reads_this_cycle > 0 && RQ.has_ready() && std::size(MSHR) != MSHR_SIZE) {
    PACKET& handle_pkt = RQ.front();

    DP(if (warmup_complete[handle_pkt.cpu]) {
      std::cout << "[" << NAME << "] " << __func__ << " instr_id: " << handle_pkt.instr_id;
      std::cout << " address: " << std::hex << (handle_pkt.address >> LOG2_PAGE_SIZE) << " full_addr: " << handle_pkt.address;
      std::cout << " full_v_addr: " << handle_pkt.v_address;
      std::cout << " data: " << handle_pkt.data << std::dec;
      std::cout << " translation_level: " << +handle_pkt.translation_level;
      std::cout << " event: " << handle_pkt.event_cycle << " current: " << current_cycle << std::endl;
    });

    auto ptw_addr = splice_bits(CR3_addr, vmem.get_offset(handle_pkt.address, vmem.pt_levels - 1) * PTE_BYTES, LOG2_PAGE_SIZE);

		auto ptw_level = vmem.pt_levels - 1;
    for (auto pscl : {&PSCL5, &PSCL4, &PSCL3, &PSCL2}) {
      if (auto check_addr = pscl->check_hit(handle_pkt.address); check_addr.has_value()) {
        ptw_addr = check_addr.value();
        ptw_level = pscl->level - 1;
      }
    }

    PACKET packet = handle_pkt;
    packet.fill_level = lower_level->fill_level; // This packet will be sent from L1 to PTW.
    packet.address = ptw_addr;
    packet.v_address = handle_pkt.address;
    packet.cpu = cpu;
    packet.type = TRANSLATION;
    packet.init_translation_level = ptw_level;
    packet.translation_level = packet.init_translation_level;
    packet.to_return = {this};

#ifdef MULTIPLE_PAGE_SIZE
		packet.page_size = handle_pkt.page_size;
		packet.base_vpn = handle_pkt.base_vpn;
#endif

		DP( std::cout << "- [PTW_HANDLE_FILL] NEW PACKET -" << std::endl;
				std::cout << "fill_level:" << setw(10) << (uint32_t)packet.fill_level << std::endl;
				std::cout << "   address:" << setw(10) << std::hex << packet.address << std::dec << std::endl;
				std::cout << "  vaddress:" << setw(10) << std::hex << packet.v_address << std::dec << std::endl;
				std::cout << "       cpu:" << setw(10) << packet.cpu << std::endl;
				std::cout << "      type:" << setw(10) << (uint32_t)packet.type << std::endl;
#ifdef MULTIPLE_PAGE_SIZE
				std::cout << " page_size:" << setw(10) << packet.page_size << std::endl;
				std::cout << "  base_vpn:" << setw(10) << packet.base_vpn << std::endl;
#endif
			);

    int rq_index = lower_level->add_rq(&packet);
    if (rq_index == -2)
      return;

    packet.to_return = handle_pkt.to_return; // Set the return for MSHR packet same as read packet.
    packet.type = handle_pkt.type;

    auto it = MSHR.insert(std::end(MSHR), packet);
    it->cycle_enqueued = current_cycle;
    it->event_cycle = std::numeric_limits<uint64_t>::max();

    RQ.pop_front();
    reads_this_cycle--;
  }
}

void PageTableWalker::handle_fill()
{
  DP(if (warmup_complete[0]) std::cout << "[" << NAME << "::handle_fill]" << std::endl; );
  int fill_this_cycle = MAX_FILL;

  while (fill_this_cycle > 0 && !std::empty(MSHR) && MSHR.front().event_cycle <= current_cycle) {
    auto fill_mshr = MSHR.begin();
/*
		DP( std::cout << "PAGE_SIZE:" << fill_mshr->page_size << "\n"; );
		DP( std::cout << "Address:" << std::hex << fill_mshr->address << "\n"; );
		DP( std::cout << "V_address:" << std::hex << fill_mshr->v_address << "\n"; );
		DP( std::cout << "translation_level:" << std::hex << (uint32_t)fill_mshr->translation_level << "\n"; );
		DP( std::cout << "PSCL5.level:" << (uint32_t)PSCL5.level << std::endl; );
		DP( std::cout << "PSCL4.level:" << (uint32_t)PSCL4.level << std::endl; );
		DP( std::cout << "PSCL3.level:" << (uint32_t)PSCL3.level << std::endl; );
		DP( std::cout << "PSCL2.level:" << (uint32_t)PSCL2.level << std::endl; );
*/

#ifdef MULTIPLE_PAGE_SIZE
		if (fill_mshr->translation_level == 1 && fill_mshr->page_size == LARGE_PAGE_SIZE) {
			DP( std::cout << "Large page found, skipping PSCL5" << std::endl; );
			fill_mshr->translation_level--;
		}
#endif

    if (fill_mshr->translation_level == 0) // If translation complete
    {
      // Return the translated physical address to STLB. Does not contain last
      // 12 bits
#ifdef MULTIPLE_PAGE_SIZE
			DP( std::cout << "FILL_PTW: PAGE_SIZE=" << fill_mshr->page_size << std::endl; );
      auto [addr, fault] = vmem.va_to_pa(cpu, fill_mshr->v_address, lg2(fill_mshr->page_size));
#else
      auto [addr, fault] = vmem.va_to_pa(cpu, fill_mshr->v_address, LOG2_PAGE_SIZE);
#endif
	
			if (warmup_complete[cpu] && fault) {
				DP ( std::cout << "Page_Fault" << std::endl; );
        fill_mshr->event_cycle = current_cycle + vmem.minor_fault_penalty;
        MSHR.sort(ord_event_cycle<PACKET>{});
      } else {
        fill_mshr->data = addr;
        fill_mshr->address = fill_mshr->v_address;
				DP( std::cout << "Page Found" << std::endl;);
        DP(if (warmup_complete[fill_mshr->cpu] || 1) {
          std::cout << "[" << NAME << "] " << __func__ << " instr_id: " << fill_mshr->instr_id;
          std::cout << " address: " << std::hex << (fill_mshr->address >> LOG2_PAGE_SIZE) << " full_addr: " << fill_mshr->address;
          std::cout << " full_v_addr: " << fill_mshr->v_address;
          std::cout << " data: " << fill_mshr->data << std::dec;
          std::cout << " translation_level: " << +fill_mshr->translation_level;
          std::cout << " index: " << std::distance(MSHR.begin(), fill_mshr) << " occupancy: " << get_occupancy(0, 0);
          std::cout << " event: " << fill_mshr->event_cycle << " current: " << current_cycle << std::endl;
        });

        for (auto ret : fill_mshr->to_return)
          ret->return_data(&(*fill_mshr));

        if (warmup_complete[cpu])
          total_miss_latency += current_cycle - fill_mshr->cycle_enqueued;

        MSHR.erase(fill_mshr);
      }
    } else {
      auto [addr, fault] = vmem.get_pte_pa(cpu, fill_mshr->v_address, fill_mshr->translation_level);
      if (warmup_complete[cpu] && fault) {
        fill_mshr->event_cycle = current_cycle + vmem.minor_fault_penalty;
        MSHR.sort(ord_event_cycle<PACKET>{});
      } else {
        if (fill_mshr->translation_level == PSCL5.level) {
					DP( std::cout << "PSCL5" << std::endl; );
          PSCL5.fill_cache(addr, fill_mshr->v_address);
				}
        if (fill_mshr->translation_level == PSCL4.level) {
					DP( std::cout << "PSCL4" << std::endl; );
          PSCL4.fill_cache(addr, fill_mshr->v_address);
				}
				if (fill_mshr->translation_level == PSCL3.level) {
					DP( std::cout << "PSCL3" << std::endl; );
          PSCL3.fill_cache(addr, fill_mshr->v_address);

				}
        if (fill_mshr->translation_level == PSCL2.level && 0) {
						DP( std::cout << "PSCL2" << std::endl; );
						PSCL2.fill_cache(addr, fill_mshr->v_address);
				}

        DP(if (warmup_complete[fill_mshr->cpu] || 1) {
          std::cout << "[" << NAME << "] " << __func__ << " instr_id: " << fill_mshr->instr_id;
#ifdef MULTIPLE_PAGE_SIZE
          std::cout << " address: " << std::hex << (fill_mshr->address >> lg2(fill_mshr->page_size)) << " full_addr: " << fill_mshr->address << " (" << fill_mshr->page_size << ")";
#else
					std::cout << " address: " << std::hex << (fill_mshr->address >> LOG2_PAGE_SIZE) << " full_addr: " << fill_mshr->address;
#endif
					std::cout << " full_v_addr: " << fill_mshr->v_address;
          std::cout << " data: " << fill_mshr->data << std::dec;
          std::cout << " translation_level: " << +fill_mshr->translation_level;
          std::cout << " index: " << std::distance(MSHR.begin(), fill_mshr) << " occupancy: " << get_occupancy(0, 0);
          std::cout << " event: " << fill_mshr->event_cycle << " current: " << current_cycle << std::endl;
        });

        PACKET packet = *fill_mshr;
        packet.cpu = cpu;
        packet.type = TRANSLATION;
        packet.address = addr;
        packet.to_return = {this};
        packet.translation_level = fill_mshr->translation_level - 1;

#ifdef MULTIPLE_PAGE_SIZE
				packet.page_size = fill_mshr->page_size;
				packet.base_vpn = fill_mshr->base_vpn;
#endif

/*
				DP( std::cout << "- [PTW_HANDLE_FILL] NEW PACKET -" << std::endl;
						std::cout << "fill_level:" << setw(10) << (uint32_t)packet.fill_level << std::endl;
						std::cout << "   address:" << setw(10) << std::hex << packet.address << std::dec << std::endl;
						std::cout << "  vaddress:" << setw(10) << std::hex << packet.v_address << std::dec << std::endl;
						std::cout << "       cpu:" << setw(10) << packet.cpu << std::endl;
						std::cout << "      type:" << setw(10) << (uint32_t)packet.type << std::endl;
						std::cout << " page_size:" << setw(10) << packet.page_size << std::endl;
						std::cout << "  base_vpn:" << setw(10) << packet.base_vpn << std::endl;
					);
*/

				int rq_index = lower_level->add_rq(&packet); // Don't get what's going on here, is this a check for successful read?
        if (rq_index != -2) {
          fill_mshr->event_cycle = std::numeric_limits<uint64_t>::max();
          fill_mshr->address = packet.address;
          fill_mshr->translation_level--;

          MSHR.splice(std::end(MSHR), MSHR, fill_mshr);
        }
      }
    }

    fill_this_cycle--;
  }
}

void PageTableWalker::operate()
{
  handle_fill();
  handle_read();
  RQ.operate();
}

int PageTableWalker::add_rq(PACKET* packet)
{
  assert(packet->address != 0);

  // check for duplicates in the read queue
#ifdef MULTIPLE_PAGE_SIZE
	//FIXME: I don't understand why this doesn't work...
	//auto found_rq = std::find_if(RQ.begin(), RQ.end(), eq_addr<PACKET>(packet->address, lg2(packet->page_size)));
	//FIXME: first time page_size == 0, maybe worth checking if this is ok
	//std::cout << "lg2 page size: " << lg2(packet->page_size) << endl;
  auto found_rq = std::find_if(RQ.begin(), RQ.end(), eq_addr<PACKET>(packet->address, LOG2_PAGE_SIZE));
#else
  auto found_rq = std::find_if(RQ.begin(), RQ.end(), eq_addr<PACKET>(packet->address, LOG2_PAGE_SIZE));
#endif
	assert(found_rq == RQ.end()); // Duplicate request should not be sent.

  // check occupancy
  if (RQ.full()) {
    return -2; // cannot handle this request
  }

  // if there is no duplicate, add it to RQ
  RQ.push_back(*packet);

  return RQ.occupancy();
}

void PageTableWalker::return_data(PACKET* packet)
{
  for (auto& mshr_entry : MSHR) {
    if (eq_addr<PACKET>{packet->address, LOG2_BLOCK_SIZE}(mshr_entry)) {
      mshr_entry.event_cycle = current_cycle;

      DP(if (warmup_complete[cpu]) {
        std::cout << "[" << NAME << "_MSHR] " << __func__ << " instr_id: " << mshr_entry.instr_id;
        std::cout << " address: " << std::hex << mshr_entry.address;
        std::cout << " v_address: " << mshr_entry.v_address;
        std::cout << " data: " << mshr_entry.data << std::dec;
        std::cout << " translation_level: " << +mshr_entry.translation_level;
        std::cout << " occupancy: " << get_occupancy(0, mshr_entry.address);
        std::cout << " event: " << mshr_entry.event_cycle << " current: " << current_cycle << std::endl;
      });
    }
  }

  MSHR.sort(ord_event_cycle<PACKET>());
}

uint32_t PageTableWalker::get_occupancy(uint8_t queue_type, uint64_t address)
{
  if (queue_type == 0)
    return std::count_if(MSHR.begin(), MSHR.end(), is_valid<PACKET>());
  else if (queue_type == 1)
    return RQ.occupancy();
  return 0;
}

uint32_t PageTableWalker::get_size(uint8_t queue_type, uint64_t address)
{
  if (queue_type == 0)
    return MSHR_SIZE;
  else if (queue_type == 1)
    return RQ.size();
  return 0;
}

void PagingStructureCache::fill_cache(uint64_t next_level_paddr, uint64_t vaddr)
{
  auto set_idx = (vaddr >> vmem.shamt(level + 1)) & bitmask(lg2(NUM_SET));
  auto set_begin = std::next(std::begin(block), set_idx * NUM_WAY);
  auto set_end = std::next(set_begin, NUM_WAY);
  auto fill_block = std::max_element(set_begin, set_end, lru_comparator<block_t, block_t>());

  *fill_block = {true, vaddr, next_level_paddr, fill_block->lru};
  std::for_each(set_begin, set_end, lru_updater<block_t>(fill_block));
}

std::optional<uint64_t> PagingStructureCache::check_hit(uint64_t address)
{
  auto set_idx = (address >> vmem.shamt(level + 1)) & bitmask(lg2(NUM_SET));
  auto set_begin = std::next(std::begin(block), set_idx * NUM_WAY);
  auto set_end = std::next(set_begin, NUM_WAY);
  auto hit_block = std::find_if(set_begin, set_end, eq_addr<block_t>{address, vmem.shamt(level + 1)});

	if (hit_block != set_end)
    return splice_bits(hit_block->data, vmem.get_offset(address, level) * PTE_BYTES, LOG2_PAGE_SIZE);

  return {};
}

void PageTableWalker::print_deadlock()
{
  if (!std::empty(MSHR)) {
    std::cout << NAME << " MSHR Entry" << std::endl;
    std::size_t j = 0;
    for (PACKET entry : MSHR) {
      std::cout << "[" << NAME << " MSHR] entry: " << j++ << " instr_id: " << entry.instr_id;
      std::cout << " address: " << std::hex << entry.address << " v_address: " << entry.v_address << std::dec << " type: " << +entry.type;
      std::cout << " translation_level: " << +entry.translation_level;
      std::cout << " fill_level: " << +entry.fill_level << " event_cycle: " << entry.event_cycle << std::endl;
    }
  } else {
    std::cout << NAME << " MSHR empty" << std::endl;
  }
}
