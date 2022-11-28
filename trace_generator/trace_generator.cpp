#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <cassert>
#include <iostream>
#include <fstream>
#include <list>

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


//int main(int argc, char *argv[]){
int main (int argc, char **argv) {

	char *output_trace_filename = NULL;
//	char *trace_extension_filename = NULL;
	int c;
	bool verbose = false;
	unsigned total_instr = 0;

	opterr = 0;

	while ((c = getopt (argc, argv, "o:v")) != -1) {
		switch (c) {
			case 'o':
				output_trace_filename = optarg;
				break;
			case 'v':
				verbose = true;
				break;
			case '?':
				if (optopt == 'o')
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


	printf("Generating memory sequence trace %s...\n", output_trace_filename);

	// open output trace and initilize boost compression filter
	std::ofstream output_trace_file(output_trace_filename, std::ios::out | std::ios::binary);
	// use boost stream filters for on-the-fly compression
	boost::iostreams::filtering_ostream write_trace_stream;
	write_trace_stream.push(boost::iostreams::lzma_compressor(boost::iostreams::lzma_params(9)));
	write_trace_stream.push(output_trace_file);

	//  file.open("test.bin", std::ios::app | std::ios::binary);
//	  file.write(reinterpret_cast<char*>(&myuint), sizeof(myuint));
	/*	
	TRACE_Extension trace_extension;
	// open output trace and initilize boost compression filter
	ofstream trace_extension_file(trace_extension.extension_string, ios_base::out | ios_base::binary);
	// use boost stream filters for on-the-fly compression
	boost::iostreams::filtering_ostream write_extension_stream;
	write_extension_stream.push(boost::iostreams::lzma_compressor(boost::iostreams::lzma_params(9)));
	write_extension_stream.push(trace_extension_file);
*/
	total_instr = 0;
	uint64_t pc = 0x4005e2;
	
	uint64_t A = 0x7ffff700b010; 
	uint64_t B = A + 4*1024;
	uint64_t C = B + 4*1024;
 	uint64_t D = C + 4*1024;
	uint64_t E = D + 4*1024;
	std::list<uint64_t> memoryAccessPattern { A, B, C, D, C, A, D, B, E, B, C, D };
	//std::list<uint64_t> memoryAccessPattern { 0xf4004, 0xf8008, 0xfb00c, 0xff010, 0xfb00c, 0xf4004, 0xff010, 0xf8008, 0x102014, 0xf8008, 0xfb00c, 0xff010 };
	std::cout << memoryAccessPattern.size() << std::endl;
	//	PageSize_info page_size_info;
	// read the trace
	for (auto address : memoryAccessPattern) {
		Output_instr instr;
		pc += 5;
		instr.ip = pc;

		// just put a read in first operand
//		current_instr.source_registers[0] = 0;
		instr.source_memory[0] = address;
//		page_size_info.page_size_source[0] = 0;
//		page_size_info.base_vpn_source[0]  = *it;

		// we don't write anywhere
		// ...

//		std::cout << std::hex << (instr.source_memory[0]) << " ";
		std::cout << std::hex << (address >> 12) << " ";
		write_trace_stream.write(reinterpret_cast<char*>(&instr), sizeof(Output_instr));
//		write_extension_stream.write((char*)&page_size_info, sizeof(PageSize_info));

		if (verbose) {
			print_instr(instr);
		}

		total_instr++;
	}
	std::cout << std::endl;
	// close the output trace
  boost::iostreams::close(write_trace_stream); // Don't forget this!
	output_trace_file.close();	
/*  
	boost::iostreams::close(write_extension_stream); // Don't forget this!
	trace_extension_file.close();	
*/
	std::cout << "Done" << std::endl;

	return 0;
}

