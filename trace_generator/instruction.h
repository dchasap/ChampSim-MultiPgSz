#define NUM_INSTR_DESTINATIONS 2
#define NUM_INSTR_SOURCES 4

class Output_instr {
	public:
		// instruction pointer or PC (Program Counter)
		uint64_t ip; 

		// branch info
		uint8_t is_branch;
		uint8_t branch_taken;

		uint8_t destination_registers[NUM_INSTR_DESTINATIONS]; // output registers
		uint8_t source_registers[NUM_INSTR_SOURCES]; // input registers

		uint64_t destination_memory[NUM_INSTR_DESTINATIONS]; // output memory
		uint64_t source_memory[NUM_INSTR_SOURCES]; // input memory

		Output_instr() {
			ip = 0;
			is_branch = 0;
			branch_taken = 0;

			for (uint32_t i=0; i<NUM_INSTR_SOURCES; i++) {
				source_registers[i] = 0;
				source_memory[i] = 0;
			}   

			for (uint32_t i=0; i<NUM_INSTR_DESTINATIONS; i++) {
				destination_registers[i] = 0;
				destination_memory[i] = 0;
			}
		};  
};

class PageSize_info {
	public:
		uint64_t base_vpn_destination[NUM_INSTR_DESTINATIONS];
    uint64_t base_vpn_source[NUM_INSTR_SOURCES];
		//TODO: we could use a char for 1 byte to minimize outputfile size
    uint8_t page_size_destination[NUM_INSTR_DESTINATIONS];
		uint8_t page_size_source[NUM_INSTR_SOURCES];

		PageSize_info()
		{
			for (uint32_t i = 0; i < NUM_INSTR_SOURCES; i++) {
				base_vpn_source[i] = 0;
        page_size_source[i]=0;
			}

      for (uint32_t i = 0; i < NUM_INSTR_DESTINATIONS; i++) {
				base_vpn_destination[i] = 0;
				page_size_destination[i]=0;
      }
		};
};

unsigned int instr_id = 0;

void print_instr(Output_instr &instr) 
{
	std::cout << "*** " << instr_id << " ***" << std::endl;
	std::cout << std::hex << "pc: 0x" << (uint64_t)instr.ip << std::dec << std::endl;
	std::cout << "is_branch:" << (uint32_t)instr.is_branch << ", is taken:" << (uint32_t)instr.branch_taken << std::endl;
	std::cout << "     source_registers:  ";

	for(uint32_t i=0; i < NUM_INSTR_SOURCES; i++)
		std::cout << (uint32_t)instr.source_registers[i] << " ";
	std::cout << std::endl;
	std::cout << "        source_memory:  ";

	for(uint32_t i=0; i < NUM_INSTR_SOURCES; i++) 
		std::cout << std::hex << "0x" << (uint32_t)instr.source_memory[i] << std::dec << " ";
	std::cout << std::endl;
		
	std::cout << "destination_registers:  ";
	for(uint32_t i=0; i < NUM_INSTR_DESTINATIONS; i++)
		std::cout << (uint32_t)instr.destination_registers[i] << " ";
	std::cout << std::endl;

	std::cout << "   destination_memory:  ";
	for(uint32_t i=0; i < NUM_INSTR_DESTINATIONS; i++)
		std::cout << std::hex << "0x" << (uint32_t)instr.destination_memory[i] << std::dec << " ";
	std::cout << std::endl;

	std::cout << std::endl;
	std::cout << std::endl; 

	instr_id++;	
}

void print_page_size_info(PageSize_info &pgsz_info) 
{
	std::cout << "*** " << instr_id << " ***" << std::endl;

	std::cout << "     source_page_size:  ";
	for(uint32_t i=0; i < NUM_INSTR_SOURCES; i++) 
		std::cout << (uint32_t)pgsz_info.page_size_source[i] << " ";
	std::cout << std::endl;

	std::cout << "      source_base_vpn:  ";
	for(uint64_t i=0; i < NUM_INSTR_SOURCES; i++) 
		std::cout << std::hex << "0x" << (uint64_t)pgsz_info.base_vpn_source[i] << std::dec << " ";
	std::cout << std::endl;

	std::cout << "destination_page_size:  ";
	for(uint32_t i=0; i < NUM_INSTR_DESTINATIONS; i++)
			std::cout << (uint32_t)pgsz_info.page_size_destination[i] << " ";
	std::cout << std::endl;

	std::cout << " destination_base_vpn:  ";
	for(uint32_t i=0; i < NUM_INSTR_DESTINATIONS; i++)
		std::cout << std::hex << "0x" << (uint64_t)pgsz_info.base_vpn_destination[i] << std::dec << " ";
	std::cout << std::endl;
	std::cout << std::endl; 

	instr_id++;	
}






