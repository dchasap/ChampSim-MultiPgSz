#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <cassert>
#include <iostream>
#include <fstream>
#include <map>

#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/filter/gzip.hpp>
#include <boost/iostreams/filter/zlib.hpp>
#include <boost/iostreams/filter/lzma.hpp>

#include "trace.h"
#include "instruction.h"

#define DEBUG_PRINT(x)
//#define DEBUG_PRINT(x) printf(x)

// number of contiguous 4KB pages that form a 2MB page
//#define CONTIGUOUS 550
//#define CONTIGUOUS 256

using namespace std;

unsigned int instr_id = 0;

void print_instr(Input_instr &instr, PageSize_info &pgsz_info) 
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
		
	std::cout << "     source_page_size:  ";
	for(uint32_t i=0; i < NUM_INSTR_SOURCES; i++) 
		std::cout << (uint32_t)pgsz_info.page_size_source[i] << " ";
	std::cout << std::endl;

	std::cout << "      source_base_vpn:  ";
	for(uint64_t i=0; i < NUM_INSTR_SOURCES; i++) 
		std::cout << std::hex << "0x" << (uint64_t)pgsz_info.base_vpn_source[i] << std::dec << " ";
	std::cout << std::endl;

	std::cout << "destination_registers:  ";
	for(uint32_t i=0; i < NUM_INSTR_DESTINATIONS; i++)
		std::cout << (uint32_t)instr.destination_registers[i] << " ";
	std::cout << std::endl;

	std::cout << "   destination_memory:  ";
	for(uint32_t i=0; i < NUM_INSTR_DESTINATIONS; i++)
		std::cout << std::hex << "0x" << (uint32_t)instr.destination_memory[i] << std::dec << " ";
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

//int main(int argc, char *argv[]){
int main (int argc, char **argv) {

	char *input_trace_filename = NULL;
	char *output_trace_filename = NULL;
	unsigned int CONTIGUOUS = 0;
	int c;
	bool verbose = false;
	bool ENABLE_LIMIT_INSTR = false;
	unsigned int INSTR_LIMIT = 0, total_instr = 0;

	opterr = 0;

	while ((c = getopt (argc, argv, "i:o:c:vl:")) != -1) {
		switch (c) {
			case 'i':
				input_trace_filename = optarg;
				break;
			case 'o':
				output_trace_filename = optarg;
				break;
			case 'c':
				CONTIGUOUS = strtoul(optarg, NULL, 0);
				printf("Contiguity set to %lu\n", CONTIGUOUS);
				break;
			case 'l':
				INSTR_LIMIT = strtoul(optarg, NULL, 0);
				ENABLE_LIMIT_INSTR = true;
				break;
			case 'v':
				verbose = true;
				break;
			case '?':
				if (optopt == 'i' or optopt == 'o')
					fprintf (stderr, "Option -%c requires an argument.\n", optopt);
				else if (isprint (optopt))
					fprintf (stderr, "Unknown option `-%c'.\n", optopt);
				else
					fprintf (stderr, "Unknown option character `\\x%x'.\n", optopt);
					return 1;
				default:
					exit (1);
		}			
	}

	CMP_TRACE input_trace;

	printf("Processing %s...\n", input_trace_filename);

	sprintf(input_trace.trace_string, "%s", input_trace_filename);
	sprintf(input_trace.gunzip_command, "xz -dc %s", input_trace_filename);	

	// open the trace(s)
	input_trace.trace_file = popen(input_trace.gunzip_command, "r");
	if (input_trace.trace_file == NULL) {
		printf("\n*** Trace file not found: %s ***\n\n", input_trace_filename);
		return 0;
	}

	// Current ChampSim instruction (without the page size)
	size_t instr_size = sizeof(Input_instr);
	Input_instr current_instr;

	// flag to ensure valid trace reading | identifier of the trace we are using since we have only one | return value when writing to the output trace
	int continue_reading = 1, retVal = 0; 

	// c++ maps
	map<uint64_t, uint64_t> page_footprint; // (vpn, #occurences)
	map<uint64_t, uint64_t> page_sizes;     // (vpn, page_size)
	map<uint64_t, uint64_t> base_page;     // (vpn, base_4kb_vpn)

	// read the trace
	while (continue_reading) {
		if (!fread(&current_instr, instr_size, 1, input_trace.trace_file)){
			// end of trace; stop reading 
			continue_reading = 0; 
		}
		else if (ENABLE_LIMIT_INSTR && total_instr >= INSTR_LIMIT) {
			continue_reading = 0;
		}
		else {
			// if ip equals zero, then there is something weird happening 
			if (current_instr.ip == 0)
				assert(0);

			// valid new instruction to process 
			for(int k = 0; k < NUM_INSTR_SOURCES; k++) {
				if(current_instr.source_memory[k] != 0)
					page_footprint[(current_instr.source_memory[k] >> 12)]++;
			}

			for(int k = 0; k < NUM_INSTR_DESTINATIONS; k++) {
				if(current_instr.destination_memory[k] != 0)
					page_footprint[(current_instr.destination_memory[k] >> 12)]++;
			}

			// just for checking the correctness fast --- debugging
			//continue_reading++;
			//if (continue_reading == INST_TO_CHECK)
			//	continue_reading = 0;
			total_instr++;
		}
	}

	// close the trace(s)
	pclose(input_trace.trace_file);

	/**********************************************************************************************************/
	/*************************** Post-processing the VPNs to identify 2MB pages *******************************/
	/**********************************************************************************************************/

	// previous vpn | count the current memory footprint | count the number of pages on a region
	uint64_t page_footprint_temp = 0, count_strides = 0, count_pages = 0;

	auto page_footprint_it = page_footprint.begin();	// first element of the c++ map
	auto page_footprint_head_it = page_footprint_it;	// head of a 2MB page
	page_footprint_temp = page_footprint_it->first;		// always stores the previous vpn in c++ map

	// move to the second vpn of the c++ map
	++page_footprint_it; 

	while(page_footprint_it != page_footprint.end()){
		// c++ maps are by default sorted  
		if (page_footprint_it->first < page_footprint_temp)
			assert(0);

		// accumulated memory footprint of a region
		count_strides += (page_footprint_it->first - page_footprint_temp);
		std::cout << "page_footprint_1:" << std::hex << page_footprint_it->first << std::dec << std::endl;
		std::cout << "page_footprint_2:" << std::hex << page_footprint_temp << std::dec << std::endl;
		std::cout << "page stride:" << (page_footprint_it->first - page_footprint_temp) << std::endl;
		std::cout << "count_strides:" << count_strides << std::endl;
		// number of pages in a given region 
		count_pages++;
		std::cout << "count_pages:" << count_pages << std::endl;

		// one 2MB page can afford up to 512 4KB pages
		if (count_strides >= 512){
			if (count_pages >= CONTIGUOUS){
				// considered as 2MB page 

				// reset the counters 
				count_pages = 0;
				count_strides = 0;

				// assign the 2mb bit to the corresponding pages + annottate the base 2MB vpn
				for(auto it = page_footprint_head_it; it!=page_footprint_it; ++it) {
					// put the base vpn to all vpns in a 2MB region
					base_page[it->first] = page_footprint_head_it->first; 					
					page_sizes[it->first] = 1;
				}

				// continue to the next region
				page_footprint_head_it = page_footprint_it;
				page_footprint_temp = page_footprint_it->first;
				++page_footprint_it;
			}
			else {
				// reset the counters
				count_pages = 0;
				count_strides = 0;

				// this is a 4KB page, so the base is the same 
				base_page[page_footprint_head_it->first] = page_footprint_head_it->first;
				page_sizes[page_footprint_head_it->first] = 0;

				// continue to the next region
				++page_footprint_head_it;
				page_footprint_temp = page_footprint_head_it->first;
				page_footprint_it = page_footprint_head_it;
				++page_footprint_it;
			}
		}
		else {
			// go to the next vpn
			page_footprint_temp = page_footprint_it->first;
			++page_footprint_it;
		}
	}

	/* last VPNs require special treatment */
	page_footprint_it = page_footprint_head_it;
	if (((double) count_pages / count_strides) > 0.2) {
		for (int i = 0; i <= count_pages; ++i) {
			base_page[page_footprint_it->first] = page_footprint_head_it->first;
			page_sizes[page_footprint_it->first] = 1;
			++page_footprint_it;
		}
	}
	else{
		for (int i=0; i<=count_pages; ++i) {
			base_page[page_footprint_head_it->first] = page_footprint_head_it->first;
			page_sizes[page_footprint_head_it->first] = 0;
			++page_footprint_head_it;
		}
	}

	// checking for bugs 
	for (auto it1 = page_footprint.cbegin(), it2 = page_sizes.cbegin(), it3 = base_page.cbegin(); it1 != page_footprint.cend() || it2 != page_sizes.cend() || it3 != base_page.cend(); it1++, it2++, it3++){
		if((it1->first == it2->first) && (it2->first == it3->first))
			;
		else{
			cout << hex << it1->first << ", " << hex << it2->first << ", " << hex << it3->first << endl;
			assert(0);
		}
	}

	/**********************************************************************************************************/
	/***************************   Generating trace extension with page sizes   *******************************/
	/**********************************************************************************************************/

	printf("Generating trace extension %s...\n", output_trace_filename);

	TRACE_Extension trace_extension;
	sprintf(trace_extension.extension_string, "%s", output_trace_filename);
	sprintf(trace_extension.compress_command, "xz -9 -f %s", output_trace_filename);	

	// and open again the trace 
	input_trace.trace_file = popen(input_trace.gunzip_command, "r");
	if (input_trace.trace_file == NULL) {
		printf("\n*** Trace file not found: %s ***\n\n", input_trace_filename);
		return 0;
	}

	// open output trace and initilize boost compression filter
	ofstream trace_extension_file(trace_extension.extension_string, ios_base::out | ios_base::binary);
	// use boost stream filters for on-the-fly compression
	boost::iostreams::filtering_ostream write_stream;
	write_stream.push(boost::iostreams::lzma_compressor(boost::iostreams::lzma_params(9)));
	write_stream.push(trace_extension_file);
	//trace_extension.extension_file = fopen(trace_extension.extension_string, "w");
	
	uint64_t temp_vpn=0;

	continue_reading = 1;
	retVal = 0;
	total_instr = 0;

	// read the trace
	while (continue_reading) {
		if (!fread(&current_instr, instr_size, 1, input_trace.trace_file)){
			// end of trace; stop reading 
			continue_reading = 0;
		}
		else if (ENABLE_LIMIT_INSTR && total_instr >= INSTR_LIMIT) {
			continue_reading = 0;
		}
		else {
			// if ip equals zero, then there is something weird happening 
			if (current_instr.ip == 0)
				assert(0);

			PageSize_info page_size_info;

			// valid new instruction to process 
			temp_vpn = 0;
			for( int k=0; k < NUM_INSTR_SOURCES; k++){
				if (current_instr.source_memory[k] != 0){
					temp_vpn = (current_instr.source_memory[k] >> 12);
					page_size_info.page_size_source[k] = page_sizes[temp_vpn];
					page_size_info.base_vpn_source[k]  = base_page[temp_vpn];
				}
			}


			for (int k = 0; k < NUM_INSTR_DESTINATIONS; k++) {
				if(current_instr.destination_memory[k] != 0){
					temp_vpn = (current_instr.destination_memory[k] >> 12);	
					page_size_info.page_size_destination[k] = page_sizes[temp_vpn];
					page_size_info.base_vpn_destination[k]  = base_page[temp_vpn];
				}
			}
			//retVal = fwrite(&page_size_info, sizeof(PageSize_info), 1, trace_extension.extension_file);
			write_stream.write((char*)&page_size_info, sizeof(PageSize_info));
			total_instr++;

			if (verbose)
				print_instr(current_instr, page_size_info);
		}
	}

	// close the input trace
	pclose(input_trace.trace_file);

	// close the output trace
	//fclose(trace_extension.extension_file);
  boost::iostreams::close(write_stream); // Don't forget this!
	trace_extension_file.close();	

	// print number of large and small pages
	unsigned int total_large_pages;
	unsigned int total_small_pages;
	for (auto it = page_sizes.begin(); it != page_sizes.end(); ++it) {
		if (it->second == 0) total_small_pages++;
		else total_large_pages++;
	}

	printf("Total small pages: %u\n", total_small_pages);
	printf("Total large pages: %u\n", total_large_pages);

	return 0;
}

