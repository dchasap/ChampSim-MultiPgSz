#include "vmem.h"

#include <algorithm>
#include <cassert>
#include <iostream>
#include <numeric>
#include <random>

//#include "champsim.h" MULTIPLE_PAGE_SIZE moved to vmem.h
#include "util.h"

#ifdef MULTIPLE_PAGE_SIZE 
// We don't really track capacity correctly, we double it and have a different list of free pages for each page size
// probably we should implement something more dynamic
VirtualMemory::VirtualMemory(uint64_t capacity, uint64_t small_pg_size,  uint64_t large_pg_size, uint32_t page_table_levels, uint64_t random_seed, uint64_t minor_fault_penalty)
    : minor_fault_penalty(minor_fault_penalty), pt_levels(page_table_levels), 
			small_page_size(small_pg_size), large_page_size(large_pg_size)
{
	uint64_t small_page_capacity = capacity / 2;
  assert(small_page_capacity % small_page_size == 0);
  assert(small_pg_size == (1ul << lg2(small_pg_size)) && small_pg_size > 1024);

	uint64_t large_page_capacity = capacity - small_page_capacity;
  assert(large_page_capacity % large_page_size == 0);
  assert(large_pg_size == (1ul << lg2(large_pg_size)) && large_pg_size > 1024);

  small_ppage_free_list = std::deque<uint64_t>((small_page_capacity - VMEM_RESERVE_CAPACITY) / small_pg_size, small_pg_size);
  large_ppage_free_list = std::deque<uint64_t>((large_page_capacity) / large_pg_size, large_pg_size);


  // populate the small page free list
  small_ppage_free_list.front() = VMEM_RESERVE_CAPACITY;
  std::partial_sum(std::cbegin(small_ppage_free_list), std::cend(small_ppage_free_list), std::begin(small_ppage_free_list));

  // then shuffle it
  std::shuffle(std::begin(small_ppage_free_list), std::end(small_ppage_free_list), std::mt19937_64{random_seed});

  next_pte_page = small_ppage_free_list.front();
  small_ppage_free_list.pop_front();
	
	// populate the large page free list
  std::partial_sum(std::cbegin(large_ppage_free_list), std::cend(large_ppage_free_list), std::begin(large_ppage_free_list));

  // then shuffle it
  std::shuffle(std::begin(large_ppage_free_list), std::end(large_ppage_free_list), std::mt19937_64{random_seed});
}

uint64_t VirtualMemory::shamt(uint32_t level) const { return lg2(small_page_size) + lg2(small_page_size / PTE_BYTES) * (level); }

uint64_t VirtualMemory::get_offset(uint64_t vaddr, uint32_t level) const { return (vaddr >> shamt(level)) & bitmask(lg2(small_page_size / PTE_BYTES)); }

std::pair<uint64_t, bool> VirtualMemory::va_to_pa(uint32_t cpu_num, uint64_t vaddr, uint32_t page_size)
{
	uint64_t free_page = ((page_size == small_page_size)?small_ppage_free_list.front():large_ppage_free_list.front());
  auto [ppage, fault] = vpage_to_ppage_map.insert({{cpu_num, vaddr >> lg2(page_size)}, free_page});

  // this vpage doesn't yet have a ppage mapping
  if (fault) {
		if (page_size == small_page_size)
			small_ppage_free_list.pop_front();
		else
			large_ppage_free_list.pop_front();
	}

  return {splice_bits(ppage->second, vaddr, lg2(page_size)), fault};
}

// MULTIPLE_PAGE_SIZE: does it matter if we leave PAs at 4KB???
// the returned PA is only used to for pte in the page table caches
std::pair<uint64_t, bool> VirtualMemory::get_pte_pa(uint32_t cpu_num, uint64_t vaddr, uint32_t level)
{
  std::tuple key{cpu_num, vaddr >> shamt(level + 1), level};
  auto [ppage, fault] = page_table.insert({key, next_pte_page});

  // this PTE doesn't yet have a mapping
  if (fault) {
    next_pte_page += small_page_size;
    if (next_pte_page % small_page_size) {
      next_pte_page = small_ppage_free_list.front();
      small_ppage_free_list.pop_front();
    }
  }

  return {splice_bits(ppage->second, get_offset(vaddr, level) * PTE_BYTES, lg2(small_page_size)), fault};
}

#else 

VirtualMemory::VirtualMemory(uint64_t capacity, uint64_t pg_size, uint32_t page_table_levels, uint64_t random_seed, uint64_t minor_fault_penalty)
    : minor_fault_penalty(minor_fault_penalty), pt_levels(page_table_levels), page_size(pg_size),
      ppage_free_list((capacity - VMEM_RESERVE_CAPACITY) / PAGE_SIZE, PAGE_SIZE)
{
  assert(capacity % PAGE_SIZE == 0);
  assert(pg_size == (1ul << lg2(pg_size)) && pg_size > 1024);

  // populate the free list
  ppage_free_list.front() = VMEM_RESERVE_CAPACITY;
  std::partial_sum(std::cbegin(ppage_free_list), std::cend(ppage_free_list), std::begin(ppage_free_list));

  // then shuffle it
  std::shuffle(std::begin(ppage_free_list), std::end(ppage_free_list), std::mt19937_64{random_seed});

  next_pte_page = ppage_free_list.front();
  ppage_free_list.pop_front();
}

uint64_t VirtualMemory::shamt(uint32_t level) const { return LOG2_PAGE_SIZE + lg2(page_size / PTE_BYTES) * (level); }

uint64_t VirtualMemory::get_offset(uint64_t vaddr, uint32_t level) const { return (vaddr >> shamt(level)) & bitmask(lg2(page_size / PTE_BYTES)); }

std::pair<uint64_t, bool> VirtualMemory::va_to_pa(uint32_t cpu_num, uint64_t vaddr, uint32_t log2_page_size)
{
  auto [ppage, fault] = vpage_to_ppage_map.insert({{cpu_num, vaddr >> log2_page_size}, ppage_free_list.front()});

  // this vpage doesn't yet have a ppage mapping
  if (fault)
    ppage_free_list.pop_front();

  return {splice_bits(ppage->second, vaddr, log2_page_size), fault};
}

// MULTIPLE_PAGE_SIZE: does it matter if we leave PAs at 4KB???
// the returned PA is only used to for pte in the page table caches
std::pair<uint64_t, bool> VirtualMemory::get_pte_pa(uint32_t cpu_num, uint64_t vaddr, uint32_t level)
{
  std::tuple key{cpu_num, vaddr >> shamt(level + 1), level};
  auto [ppage, fault] = page_table.insert({key, next_pte_page});

  // this PTE doesn't yet have a mapping
  if (fault) {
    next_pte_page += page_size;
    if (next_pte_page % page_size) {
      next_pte_page = ppage_free_list.front();
      ppage_free_list.pop_front();
    }
  }

  return {splice_bits(ppage->second, get_offset(vaddr, level) * PTE_BYTES, lg2(page_size)), fault};
}
#endif
