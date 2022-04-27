#include <algorithm>
#include <iterator>

#include "cache.h"
#include "util.h"

#define MAX_EVICTION_CANDIDATES 6 

bool block_compare(const BLOCK a, const BLOCK b){
	    return a.lru < b.lru;
}

// Warning! This policy should only be initialized for TLBs
void CACHE::initialize_replacement() {
	std::cout << "Using MPSZ_LRU" << std::endl;
}

template<class ForwardIt>
ForwardIt max_n_element(ForwardIt first, ForwardIt last, unsigned int N)
{
	    if (first == last) return last;
			 
			    ForwardIt largest = first;
					    ++first;
							    for (; first != last; ++first) {
										        if (*largest < *first) {
															            largest = first;
																					        }
														    }
									    return largest;
}
// find replacement victim
uint32_t CACHE::find_victim(uint32_t cpu, uint64_t instr_id, uint32_t set, const BLOCK* current_set, uint64_t ip, uint64_t full_addr, uint32_t type)
{
    uint32_t way = 0;

    // fill invalid line first
    for (way=0; way<NUM_WAY; way++) {
        if (block[set][way].valid == false) {

            DP ( if (warmup_complete[cpu]) {
            cout << "[" << NAME << "] " << __func__ << " instr_id: " << instr_id << " invalid set: " << set << " way: " << way;
            cout << hex << " address: " << (full_addr>>LOG2_BLOCK_SIZE) << " victim address: " << block[set][way].address << " data: " << block[set][way].data;
            cout << dec << " lru: " << block[set][way].lru << endl; });

            break;
        }
    }

		// No invalid entries
		// Now find possible candidates
		uint32_t max_small_page_lru = 0;
		uint32_t small_page_candidate_index = 0;
    // LRU victim
    if (way == NUM_WAY) {
        for (way=0; way<NUM_WAY; way++) {
						// find possible candidate
						// TODO: we should maybe have a threshold on lru distance from actual lru block
						//assert(current_set[way].page_size == 0);
						if (current_set[way].page_size == 0) { // In giorgios version page size 0 stands for base page size
								if (current_set[way].lru > max_small_page_lru) {
										max_small_page_lru = current_set[way].lru;
										small_page_candidate_index = way;
								}
						}

            if (block[set][way].lru == NUM_WAY-1) {

                DP ( if (warmup_complete[cpu]) {
                cout << "[" << NAME << "] " << __func__ << " instr_id: " << instr_id << " replace set: " << set << " way: " << way;
                cout << hex << " address: " << (full_addr>>LOG2_BLOCK_SIZE) << " victim address: " << block[set][way].address << " data: " << block[set][way].data;
                cout << dec << " lru: " << block[set][way].lru << endl; });

                break;
            }
        }

				//check if base page victim is different from lru one
				if (way != small_page_candidate_index && max_small_page_lru != 0) {
						//TODO:We should alter lru counter by -1 from end till the index of 4KB victim to adjust the policy
						//std::cout << "SMALL PAGE LRU:" << max_small_page_lru << std::endl;
						way = small_page_candidate_index;
				}
    }

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
