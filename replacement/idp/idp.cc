#include <algorithm>
#include <iterator>

#include "cache.h"
#include "util.h"

/*
 * Mode1: Just favor instr or data packets 
 */

uint32_t victim_threshold;

void CACHE::initialize_replacement() {
	
	//TODO: maybe add a check it var is set, else default to a value or exit 
	victim_threshold = stoi(getenv("REPL_IDP_VICTIM_THRESHOLD"));
	std::cout << this->NAME << " using idp replacement policy (victim threshold: " 
						<< victim_threshold << ")" << std::endl;
	// adjust the threshold acording to NUM_WAY
	assert(victim_threshold < NUM_WAY);
	victim_threshold = NUM_WAY - victim_threshold;
}

// find replacement victim
uint32_t CACHE::find_victim(uint32_t cpu, uint64_t instr_id, uint32_t set, const BLOCK* current_set, uint64_t ip, uint64_t full_addr, uint32_t type)
{
		uint32_t way = 0;
		//std::cout << "Looking for victim @set:" << set << std::endl;
    // fill invalid line first
    for (way=0; way<NUM_WAY; way++) 
        if (block[(set*NUM_WAY) + way].valid == false) 
            break; 

		// No invalid entries
		// Now find possible candidates
		uint32_t max_victim_lru = 0;
		uint32_t lru_victim_index = 0, idp_victim_index = 0;
    // LRU victim
    if (way == NUM_WAY) {
      for (way=0; way<NUM_WAY; way++) {
				// find possible candidate
				if (!current_set[way].is_instr) {
					//std::cout << "instr found@" << way << std::endl;
					// need to also check if idp victim lru value is higher than threshold value
					if (current_set[way].lru > max_victim_lru && current_set[way].lru >= victim_threshold) {
						max_victim_lru = current_set[way].lru;
						idp_victim_index = way;
						//std::cout << "setting victim index@" << way << std::endl;
						//std::cout << "(lru:" << max_victim_lru << ")" << std::endl;
					}
				}
				// this is the default behavior, way will keep the default lru victim,
				// we need to check later if victim_index should be used instead
				//std::cout << "lru:" << current_set[way].lru << std::endl;
        if (current_set[way].lru == NUM_WAY-1) {
					//std::cout << "max lru found@" << way << std::endl;
					//break;
					lru_victim_index = way;
				}

			}
			// check if preference victim is different from lru one
			if (way != idp_victim_index && max_victim_lru != 0) {
				//std::cout << "we have a candidate@" << way << std::endl;
				// We should adjust lru values that are greater than the one we evict
				for (way = 0; way < NUM_WAY; way++) {
					//std::cout << "(original lru:" << current_set[way].lru << ")" << std::endl;
					if (current_set[way].lru > current_set[idp_victim_index].lru) {
						block[(set*NUM_WAY) + way].lru--;
					}
					//std::cout << "(adjusted lru:" << current_set[way].lru << ")" << std::endl;
				}
				way = idp_victim_index;
				// don't forget to put max lru to the evicted entry
				block[(set*NUM_WAY) + way].lru = NUM_WAY - 1;
			}
			else 
				way = lru_victim_index;
    }
		//std::cout << "almost..." << std::endl;
    if (way == NUM_WAY) {
        cerr << "[" << NAME << "] " << __func__ << " no victim! set: " << set << endl;
        assert(0);
    }

		return way;
}

// called on every cache hit and cache fill
void CACHE::update_replacement_state(uint32_t cpu, uint32_t set, uint32_t way, uint64_t full_addr, uint64_t ip, uint64_t victim_addr, uint32_t type,
                                     uint8_t hit)
{
  if (hit && type == WRITEBACK)
    return;

  auto begin = std::next(block.begin(), set * NUM_WAY);
  auto end = std::next(begin, NUM_WAY);
  uint32_t hit_lru = std::next(begin, way)->lru;
  std::for_each(begin, end, [hit_lru](BLOCK& x) {
    if (x.lru <= hit_lru)
      x.lru++;
  });
  std::next(begin, way)->lru = 0; // promote to the MRU position
}

void CACHE::replacement_final_stats() {}

