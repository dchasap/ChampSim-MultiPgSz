#ifndef INSTRUCTION_H
#define INSTRUCTION_H

#include <array>
#include <cstdint>
#include <iostream>
#include <limits>
#include <vector>

#include "circular_buffer.hpp"
#include "champsim.h"
// instruction format
#define NUM_INSTR_DESTINATIONS_SPARC 4
#define NUM_INSTR_DESTINATIONS 2
#define NUM_INSTR_SOURCES 4

// special registers that help us identify branches
#define REG_STACK_POINTER 6
#define REG_FLAGS 25
#define REG_INSTRUCTION_POINTER 26

// branch types
#define NOT_BRANCH 0
#define BRANCH_DIRECT_JUMP 1
#define BRANCH_INDIRECT 2
#define BRANCH_CONDITIONAL 3
#define BRANCH_DIRECT_CALL 4
#define BRANCH_INDIRECT_CALL 5
#define BRANCH_RETURN 6
#define BRANCH_OTHER 7

class LSQ_ENTRY;

struct input_instr {
  // instruction pointer or PC (Program Counter)
  uint64_t ip = 0;

  // branch info
  uint8_t is_branch = 0;
  uint8_t branch_taken = 0;

  uint8_t destination_registers[NUM_INSTR_DESTINATIONS] = {}; // output registers
  uint8_t source_registers[NUM_INSTR_SOURCES] = {};           // input registers

  uint64_t destination_memory[NUM_INSTR_DESTINATIONS] = {}; // output memory
	uint64_t source_memory[NUM_INSTR_SOURCES] = {};           // input memory

/*
#ifdef MULTIPLE_PAGE_SIZE
	uint64_t destination_base_vpn[NUM_INSTR_DESTINATIONS];
	uint64_t source_base_vpn[NUM_INSTR_SOURCES];

	uint8_t destination_page_size[NUM_INSTR_DESTINATIONS];
	uint8_t source_page_size[NUM_INSTR_SOURCES];
#endif
*/

};

#ifdef MULTIPLE_PAGE_SIZE
struct page_size_info {

	uint64_t destination_base_vpn[NUM_INSTR_DESTINATIONS] = {};
	uint64_t source_base_vpn[NUM_INSTR_SOURCES] = {};
	uint8_t destination_page_size[NUM_INSTR_DESTINATIONS] = {};
	uint8_t source_page_size[NUM_INSTR_SOURCES] = {};
};
#endif

struct cloudsuite_instr {
  // instruction pointer or PC (Program Counter)
  uint64_t ip = 0;

  // branch info
  uint8_t is_branch = 0;
  uint8_t branch_taken = 0;

  uint8_t destination_registers[NUM_INSTR_DESTINATIONS_SPARC] = {}; // output registers
  uint8_t source_registers[NUM_INSTR_SOURCES] = {};                 // input registers

  uint64_t destination_memory[NUM_INSTR_DESTINATIONS_SPARC] = {}; // output memory
  uint64_t source_memory[NUM_INSTR_SOURCES] = {};                 // input memory

#ifdef MULTIPLE_PAGE_SIZE
	//FIXME: Not sure this would work, position of variablies are importantAAAAAAAAAAA
	uint64_t destination_base_vpn[NUM_INSTR_DESTINATIONS];
	uint64_t source_base_vpn[NUM_INSTR_SOURCES];

  uint8_t destination_page_size[NUM_INSTR_DESTINATIONS];
	uint8_t source_page_size[NUM_INSTR_SOURCES];
#endif

  uint8_t asid[2] = {std::numeric_limits<uint8_t>::max(), std::numeric_limits<uint8_t>::max()};
};

struct ooo_model_instr {
  uint64_t instr_id = 0, ip = 0, event_cycle = 0;

  bool is_branch = 0, is_memory = 0, branch_taken = 0, branch_mispredicted = 0, source_added[NUM_INSTR_SOURCES] = {},
       destination_added[NUM_INSTR_DESTINATIONS_SPARC] = {};

  uint8_t asid[2] = {std::numeric_limits<uint8_t>::max(), std::numeric_limits<uint8_t>::max()};

  uint8_t branch_type = NOT_BRANCH;
  uint64_t branch_target = 0;

  uint8_t translated = 0, fetched = 0, decoded = 0, scheduled = 0, executed = 0;
  int num_reg_ops = 0, num_mem_ops = 0, num_reg_dependent = 0;

  uint8_t destination_registers[NUM_INSTR_DESTINATIONS_SPARC] = {}; // output registers

  uint8_t source_registers[NUM_INSTR_SOURCES] = {}; // input registers

  // these are indices of instructions in the ROB that depend on me
  std::vector<champsim::circular_buffer<ooo_model_instr>::iterator> registers_instrs_depend_on_me, memory_instrs_depend_on_me;

  // memory addresses that may cause dependencies between instructions
  uint64_t instruction_pa = 0;
  uint64_t destination_memory[NUM_INSTR_DESTINATIONS_SPARC] = {}; // output memory
  uint64_t source_memory[NUM_INSTR_SOURCES] = {};                 // input memory

  std::array<std::vector<LSQ_ENTRY>::iterator, NUM_INSTR_SOURCES> lq_index = {};
  std::array<std::vector<LSQ_ENTRY>::iterator, NUM_INSTR_DESTINATIONS_SPARC> sq_index = {};

#ifdef MULTIPLE_PAGE_SIZE
  uint64_t destination_base_vpn[NUM_INSTR_DESTINATIONS];
	uint64_t source_base_vpn[NUM_INSTR_SOURCES];

	uint8_t destination_page_size[NUM_INSTR_DESTINATIONS];
	uint8_t source_page_size[NUM_INSTR_SOURCES];

	// keep around a record of what the original virtual addresses were
	uint64_t destination_virtual_address[NUM_INSTR_DESTINATIONS_SPARC];
	uint64_t source_virtual_address[NUM_INSTR_SOURCES];
#endif

  ooo_model_instr() = default;

#ifdef MULTIPLE_PAGE_SIZE
	ooo_model_instr(uint8_t cpu, input_instr instr, page_size_info pgsz_info)
#else
  ooo_model_instr(uint8_t cpu, input_instr instr)
#endif
  {
    std::copy(std::begin(instr.destination_registers), std::end(instr.destination_registers), std::begin(this->destination_registers));
    std::copy(std::begin(instr.destination_memory), std::end(instr.destination_memory), std::begin(this->destination_memory));
    std::copy(std::begin(instr.source_registers), std::end(instr.source_registers), std::begin(this->source_registers));
    std::copy(std::begin(instr.source_memory), std::end(instr.source_memory), std::begin(this->source_memory));

#ifdef MULTIPLE_PAGE_SIZE
    std::copy(std::begin(pgsz_info.destination_base_vpn), std::end(pgsz_info.destination_base_vpn), std::begin(this->destination_base_vpn));
    std::copy(std::begin(pgsz_info.destination_page_size), std::end(pgsz_info.destination_page_size), std::begin(this->destination_page_size));
		std::copy(std::begin(instr.destination_memory), std::end(instr.destination_memory), std::begin(this->destination_virtual_address));
    std::copy(std::begin(pgsz_info.source_base_vpn), std::end(pgsz_info.source_base_vpn), std::begin(this->source_base_vpn));
    std::copy(std::begin(pgsz_info.source_page_size), std::end(pgsz_info.source_page_size), std::begin(this->source_page_size));
    std::copy(std::begin(instr.source_memory), std::end(instr.source_memory), std::begin(this->source_virtual_address));
#endif

    this->ip = instr.ip;
    this->is_branch = instr.is_branch;
    this->branch_taken = instr.branch_taken;

    asid[0] = cpu;
    asid[1] = cpu;
  }

#ifdef MULTIPLE_PAGE_SIZE
  ooo_model_instr(uint8_t cpu, cloudsuite_instr instr, page_size_info pgsz_info)
#else
	ooo_model_instr(uint8_t cpu, cloudsuite_instr instr)
#endif
	{
    std::copy(std::begin(instr.destination_registers), std::end(instr.destination_registers), std::begin(this->destination_registers));
    std::copy(std::begin(instr.destination_memory), std::end(instr.destination_memory), std::begin(this->destination_memory));
    std::copy(std::begin(instr.source_registers), std::end(instr.source_registers), std::begin(this->source_registers));
    std::copy(std::begin(instr.source_memory), std::end(instr.source_memory), std::begin(this->source_memory));

#ifdef MULTIPLE_PAGE_SIZE
    std::copy(std::begin(pgsz_info.destination_base_vpn), std::end(pgsz_info.destination_base_vpn), std::begin(this->destination_base_vpn));
    std::copy(std::begin(pgsz_info.destination_page_size), std::end(pgsz_info.destination_page_size), std::begin(this->destination_page_size));
		std::copy(std::begin(instr.destination_memory), std::end(instr.destination_memory), std::begin(this->destination_virtual_address));
    std::copy(std::begin(pgsz_info.source_base_vpn), std::end(pgsz_info.source_base_vpn), std::begin(this->source_base_vpn));
    std::copy(std::begin(pgsz_info.source_page_size), std::end(pgsz_info.source_page_size), std::begin(this->source_page_size));
    std::copy(std::begin(instr.source_memory), std::end(instr.source_memory), std::begin(this->source_virtual_address));

		/*
		for(unsigned int i = 0; i < NUM_INSTR_SOURCES; i++) {
			pgsz_info.source_page_size[i] = (this->source_page_size == 0)?BASE_PAGE_SIZE:LARGE_PAGE_SIZE;
		}
		
		for(unsigned int i = 0; i < NUM_INSTR_DESTINATIONS; i++) {
			pgsz_info.destination_page_size[i] = (this->destination_page_size == 0)?BASE_PAGE_SIZE:LARGE_PAGE_SIZE;
		}	
		*/
#endif

    this->ip = instr.ip;
    this->is_branch = instr.is_branch;
    this->branch_taken = instr.branch_taken;

    std::copy(std::begin(instr.asid), std::begin(instr.asid), std::begin(this->asid));
  }

	void print()
	{
		std::cout << "*** " << this->instr_id << " ***" << std::endl;
		std::cout << std::hex << "pc: 0x" << (uint64_t)this->ip << std::dec << std::endl;
		std::cout << "is_branch:" << (uint32_t)this->is_branch << ", is taken:" << (uint32_t)this->branch_taken << std::endl;
		std::cout << "     source_registers:  ";
		for(uint32_t i=0; i < NUM_INSTR_SOURCES; i++)
			std::cout << (uint32_t)this->source_registers[i] << " ";
		std::cout << std::endl;
		std::cout << "        source_memory:  ";
		for(uint32_t i=0; i < NUM_INSTR_SOURCES; i++) 
			std::cout << std::hex << "0x" << (uint32_t)this->source_memory[i] << std::dec << " ";
		std::cout << std::endl;
#ifdef MULTIPLE_PAGE_SIZE
		std::cout << "     source_page_size:  ";
		for(uint32_t i=0; i < NUM_INSTR_SOURCES; i++) 
			std::cout << (uint32_t)this->source_page_size[i] << " ";
		std::cout << std::endl;
		std::cout << "      source_base_vpn:  ";
		for(uint64_t i=0; i < NUM_INSTR_SOURCES; i++) 
			std::cout << std::hex << "0x" << (uint64_t)this->source_base_vpn[i] << std::dec << " ";
		std::cout << std::endl;
#endif
		std::cout << "destination_registers:  ";
		for(uint32_t i=0; i < NUM_INSTR_DESTINATIONS; i++)
			std::cout << (uint32_t)this->destination_registers[i] << " ";
		std::cout << std::endl;
		std::cout << "   destination_memory:  ";
		for(uint32_t i=0; i < NUM_INSTR_DESTINATIONS; i++)
			std::cout << std::hex << "0x" << (uint32_t)this->destination_memory[i] << std::dec << " ";
		std::cout << std::endl;
#ifdef MULTIPLE_PAGE_SIZE
		std::cout << "destination_page_size:  ";
		for(uint32_t i=0; i < NUM_INSTR_DESTINATIONS; i++)
			std::cout << (uint32_t)this->destination_page_size[i] << " ";
		std::cout << std::endl;
		std::cout << " destination_base_vpn:  ";
		for(uint32_t i=0; i < NUM_INSTR_DESTINATIONS; i++)
			std::cout << std::hex << "0x" << (uint64_t)this->destination_base_vpn[i] << std::dec << " ";
		std::cout << std::endl;
#endif
		std::cout << std::endl;
	}  
};

#endif
