#include <iostream>
#include <map>
#include <assert.h>
#include "trace.h"
#include <string.h>
#include "instruction.h"

// number of contiguous 4KB pages that form a 2MB page
#define CONTIGUOUS 550

using namespace std;

int main(int argc, char *argv[]){

	CMP_TRACE traces[argc-2];

	// I assume multiple traces, but in this case we only use one trace; the for loop is actually useless
	for(int i=2; i<argc; i++){
		sprintf(traces[i-2].trace_string, "%s", argv[i]);
		sprintf(traces[i-2].gunzip_command, "xz -dc %s", argv[i]);	
	}

	// open the trace(s)
	for(int i=2; i<argc; i++){
		traces[i-2].trace_file = popen(traces[i-2].gunzip_command, "r");
		if(traces[i-2].trace_file == NULL) {
			printf("\n*** Trace file not found: %s ***\n\n", argv[i]);
			return 0;
		}
	}

	// Current ChampSim instruction (without the page size)
	size_t instr_size = sizeof(Input_instr);
	Input_instr current_instr;

	// flag to ensure valid trace reading | identifier of the trace we are using since we have only one | return value when writing to the output trace
	int continue_reading = 1, trace_id = 0, retVal = 0; 

	// c++ maps
	map<uint64_t, uint64_t> page_footprint; // (vpn, #occurences)
	map<uint64_t, uint64_t> page_sizes;     // (vpn, page_size)
	map<uint64_t, uint64_t> base_page;     // (vpn, base_4kb_vpn)

	// iterators for the above c++ maps
	map<uint64_t, uint64_t>::iterator it;  		// current 
	map<uint64_t, uint64_t>::iterator itt;  	// temp iterator 
	map<uint64_t, uint64_t>::iterator head_it;  // base vpn

	map<uint64_t, uint64_t>::iterator it1;  	// debug
	map<uint64_t, uint64_t>::iterator it2;  	// debug
	map<uint64_t, uint64_t>::iterator it3; 		// debug

	// read the trace
	while(continue_reading){
		if(!fread(&current_instr, instr_size, 1, traces[trace_id].trace_file)){
			// end of trace; stop reading 
			continue_reading = 0; 
		}
		else{
			// if ip equals zero, then there is something weird happening 
			if(current_instr.ip == 0)
				assert(0);

			// valid new instruction to process 
			for(int k=0; k<NUM_INSTR_SOURCES; k++){
				if(current_instr.source_memory[k] != 0)
					page_footprint[(current_instr.source_memory[k]>>12)]++;
			}

			for(int k=0; k<NUM_INSTR_DESTINATIONS; k++){
				if(current_instr.destination_memory[k] != 0)
					page_footprint[(current_instr.destination_memory[k]>>12)]++;
			}

			// just for checking the correctness fast --- debugging
			//continue_reading++;
			//if(continue_reading == INST_TO_CHECK)
			//	continue_reading = 0;
		}
	}

	// close the trace(s)
	for(int i=2; i<argc; i++)
		pclose(traces[i-2].trace_file);

	/**********************************************************************************************************/
	/*************************** Post-processing the VPNs to identify 2MB pages *******************************/
	/**********************************************************************************************************/

	// previous vpn | count the current memory footprint | count the number of pages on a region
	uint64_t temp = 0, count_strides = 0, count_pages = 0;

	it = page_footprint.begin(); 		  // first element of the c++ map
	temp = it->first; 			 // always stores the previous vpn in c++ map
	head_it = it;				// head of a 2MB page

	// move to the second vpn of the c++ map
	++it; 

	while(it != page_footprint.end()){
		// c++ maps are by default sorted  
		if(it->first < temp)
			assert(0);

		// accumulated memory footprint of a region
		count_strides += (it->first - temp);
		// number of pages in a given region 
		count_pages++;

		// one 2MB page can afford up to 512 4KB pages
		if(count_strides>=512){
			if(count_pages >= CONTIGUOUS){
				// considered as 2MB page 

				// reset the counters 
				count_pages = 0;
				count_strides = 0;

				// assign the 2mb bit to the corresponding pages + annottate the base 2MB vpn
				for(itt=head_it; itt!=it; itt++){
					base_page[itt->first] = head_it->first; // put the base vpn to all vpns in a 2MB region
					page_sizes[itt->first] = 1;
				}

				// continue to the next region
				head_it = it;
				temp = it->first;
				++it;
			}
			else{
				// reset the counters
				count_pages = 0;
				count_strides = 0;

				// this is a 4KB page, so the base is the same 
				base_page[head_it->first] = head_it->first;
				page_sizes[head_it->first] = 0;

				// continue to the next region
				++head_it;
				temp = head_it->first;
				it = head_it;
				++it;
			}
		}
		else{
			// go to the next vpn
			temp = it->first;
			++it;
		}
	}

	/* last VPNs require special treatment */
	it = head_it;
	if(((double)count_pages/count_strides) > 0.2){
		for(int i=0; i<=count_pages; ++i){
			base_page[it->first] = head_it->first;
			page_sizes[it->first] = 1;
			++it;
		}
	}
	else{
		for(int i=0; i<=count_pages; ++i){
			base_page[head_it->first] = head_it->first;
			page_sizes[head_it->first] = 0;
			++head_it;
		}
	}

	// checking for bugs 
	for(auto it1 = page_footprint.cbegin(), it2 = page_sizes.cbegin(), it3 = base_page.cbegin(); it1 != page_footprint.cend() || it2 != page_sizes.cend() || it3 != base_page.cend(); it1++, it2++, it3++){
		if((it1->first == it2->first) && (it2->first == it3->first))
			;
		else{
			cout << hex << it1->first << ", " << hex << it2->first << ", " << hex << it3->first << endl;
			assert(0);
		}
	}

	// and open again the trace 
	for(int i=2; i<argc; i++){
		traces[i-2].trace_file = popen(traces[i-2].gunzip_command, "r");
		if(traces[i-2].trace_file == NULL) {
			printf("\n*** Trace file not found: %s ***\n\n", argv[i]);
			return 0;
		}
	}

	string out_trace = argv[1]; //TODO
	FILE * fp = fopen(out_trace.c_str(),"w");
	//Output_instr out_instr;
	size_t out_size = sizeof(Output_instr);
	uint64_t temp_vpn=0;

	continue_reading = 1;
	trace_id = 0; 
	retVal = 0;

	// read the trace
	while(continue_reading){
		if(!fread(&current_instr, instr_size, 1, traces[trace_id].trace_file)){
			// end of trace; stop reading 
			continue_reading = 0;
		}
		else{
			// if ip equals zero, then there is something weird happening 
			if(current_instr.ip == 0)
				assert(0);

			Output_instr out_instr;

			out_instr.ip = current_instr.ip;
			out_instr.is_branch = current_instr.is_branch;
			out_instr.branch_taken = current_instr.branch_taken;

			// valid new instruction to process 
			temp_vpn = 0;
			for(int k=0; k<NUM_INSTR_SOURCES; k++){
				out_instr.source_registers[k] = current_instr.source_registers[k];
				out_instr.source_memory[k] = current_instr.source_memory[k];
				if(out_instr.source_memory[k] != 0){
					temp_vpn = (out_instr.source_memory[k]>>12);
					out_instr.page_size_source[k] = page_sizes[temp_vpn];
					out_instr.base_vpn_source[k]  = base_page[temp_vpn];
				}
			}


			for(int k=0; k<NUM_INSTR_DESTINATIONS; k++){
				out_instr.destination_registers[k] = current_instr.destination_registers[k];
				out_instr.destination_memory[k] = current_instr.destination_memory[k];
				if(out_instr.destination_memory[k] != 0){
					temp_vpn = (out_instr.destination_memory[k]>>12);	
					out_instr.page_size_destination[k] = page_sizes[temp_vpn];
					out_instr.base_vpn_destination[k]  = base_page[temp_vpn];
				}
			}

			retVal = fwrite(&out_instr, out_size, 1, fp);
		}
	}

	// close the input trace(s)
	for(int i=2; i<argc; i++)
		pclose(traces[i-2].trace_file);

	// close the output trace
	fclose(fp);
}

