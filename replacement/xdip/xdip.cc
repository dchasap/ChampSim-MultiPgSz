#include <algorithm>
#include <map>
#include <utility>

#include "cache.h"
#include "util.h"

#define maxRRPV 12
//#define NUM_POLICY 2
//#define SDM_SIZE 32
//#define TOTAL_SDM_SETS NUM_CPUS* NUM_POLICY* SDM_SIZE
//#define BIP_MAX 32
//#define PSEL_WIDTH 10
//#define PSEL_MAX ((1 << PSEL_WIDTH) - 1)
//#define PSEL_THRS PSEL_MAX / 2

//#std::map<CACHE*, unsigned> bip_counter;
//#std::map<CACHE*, std::vector<std::size_t>> rand_sets;
//#std::map<std::pair<CACHE*, std::size_t>, unsigned> PSEL;

uint32_t instr_pos, data_pos;

std::map<uint64_t, int32_t> vpn_freq_acc;

void CACHE::initialize_replacement()
{
	instr_pos = stoi(getenv("XDIP_INSTR_POS"));
	data_pos = stoi(getenv("XDIP_DATA_POS"));
	std::cout << this->NAME << " using xdim with instr@" << instr_pos 
						<< " and data@" << data_pos << std::endl;
}

// called on every cache hit and cache fill
void CACHE::update_replacement_state(uint32_t cpu, uint32_t set, uint32_t way, uint64_t full_addr, uint64_t ip, uint64_t victim_addr, uint32_t type,
                                     uint8_t hit)
{

/*
  // do not update replacement state for writebacks
  if (!block[set * NUM_WAY + way].is_instr) {
		block[set * NUM_WAY + way].lru = maxRRPV - 1;
  }
	else {
		block[set * NUM_WAY + way].lru = 0; // for cache hit, DRRIP always promotes
                                        // a cache line to the MRU position
  }
*/
/*
	// let us be kinder to data
	if (hit && block[set * NUM_WAY + way].is_instr) {
    block[set * NUM_WAY + way].lru = 0; // for cache hit, DRRIP always promotes
                                        // a cache line to the MRU position
																				//
																				//
		uint64_t vpn = block[set * NUM_WAY + way].v_address >> LOG2_PAGE_SIZE;
		vpn_freq_acc[vpn]++;
    return;
  }
*/

	// if it's data, just set the lru value an leave
	if (!block[set * NUM_WAY + way].is_instr) {
		block[set * NUM_WAY + way].lru = maxRRPV - 1;
		//block[set * NUM_WAY + way].lru =  data_pos;
		return;
	}
	
	//block[set * NUM_WAY + way].lru = instr_pos;

	uint64_t vpn = block[set * NUM_WAY + way].v_address >> LOG2_PAGE_SIZE;
	// get access frequency and setup new entry if it doesn't exit
	int32_t acc_freq = 0;
	if (vpn_freq_acc.find(vpn) == vpn_freq_acc.end()) {
		acc_freq = -2;
		vpn_freq_acc[vpn] = -2;
	} else {
		acc_freq = vpn_freq_acc[vpn];
	}

	// choose the correct placement for new block/translation
	if (acc_freq < 50) {
		block[set * NUM_WAY + way].lru = maxRRPV - 1;
	}
	else {
		block[set * NUM_WAY + way].lru = 0;
		//std::cout << maxRRPV - ((acc_freq / 50) % 4) << std::endl;
		//block[set * NUM_WAY + way].lru = maxRRPV - ((acc_freq / 50) % 4);
	}

	// update fac
	vpn_freq_acc[vpn]++;

	return;
}

// find replacement victim
uint32_t CACHE::find_victim(uint32_t cpu, uint64_t instr_id, uint32_t set, const BLOCK* current_set, uint64_t ip, uint64_t full_addr, uint32_t type)
{
  // look for the maxRRPV line
  auto begin = std::next(std::begin(block), set * NUM_WAY);
  auto end = std::next(begin, NUM_WAY);
  auto victim = std::find_if(begin, end, [](BLOCK x) { return x.lru == maxRRPV; }); // hijack the lru field
  while (victim == end) {
    for (auto it = begin; it != end; ++it)
      it->lru++;

    victim = std::find_if(begin, end, [](BLOCK x) { return x.lru == maxRRPV; });
  }

  return std::distance(begin, victim);
}

bool cmp(const pair<uint64_t, uint32_t> &a, const pair<uint64_t, uint32_t> &b) 
{
	return (a.second < b.second);
}

// use this function to print out your own stats at the end of simulation
void CACHE::replacement_final_stats() {

	std::vector <pair<uint64_t, uint32_t>> sorted_fac;
	for (auto& i : vpn_freq_acc) {
		sorted_fac.push_back(i);
	}	
	std::sort(sorted_fac.begin(), sorted_fac.end(), cmp);
	for (auto& i : sorted_fac) {
		std::cout << std::hex << i.first << std::dec;
		std::cout << ", " << i.second + maxRRPV + 1 << std::endl;
	}
}



