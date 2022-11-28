#define NUM_INSTR_DESTINATIONS 2
#define NUM_INSTR_SOURCES 4

class Input_instr{
	public:
		// instruction pointer or PC (Program Counter)
		uint64_t ip = 0; 

		// branch info
		uint8_t is_branch = 0;
		uint8_t branch_taken = 0;
  
		uint8_t destination_registers[NUM_INSTR_DESTINATIONS] = {}; // output registers
		uint8_t source_registers[NUM_INSTR_SOURCES] = {};           // input registers

		uint64_t destination_memory[NUM_INSTR_DESTINATIONS] = {}; // output memory
		uint64_t source_memory[NUM_INSTR_SOURCES] = {};           // input memory
 
};

class PageSize_info {
	public:
		uint64_t base_vpn_destination[NUM_INSTR_DESTINATIONS] = {};
    uint64_t base_vpn_source[NUM_INSTR_SOURCES] = {};
		//TODO: we could use a char for 1 byte to minimize outputfile size
    uint8_t page_size_destination[NUM_INSTR_DESTINATIONS] = {};
		uint8_t page_size_source[NUM_INSTR_SOURCES] = {};
};
