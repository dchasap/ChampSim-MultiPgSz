#include "tracereader.h"

#include <cassert>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <string>

#ifdef MULTIPLE_PAGE_SIZE
tracereader::tracereader(uint8_t cpu, std::string _ts, std::string _txs) : cpu(cpu), trace_string(_ts), trace_ext_string(_txs)
#else
tracereader::tracereader(uint8_t cpu, std::string _ts) : cpu(cpu), trace_string(_ts)
#endif
{
  std::string last_dot = trace_string.substr(trace_string.find_last_of("."));

  if (trace_string.substr(0, 4) == "http") {
    // Check file exists
    char testfile_command[4096];
    sprintf(testfile_command, "wget -q --spider %s", trace_string.c_str());
    FILE* testfile = popen(testfile_command, "r");
    if (pclose(testfile)) {
      std::cerr << "TRACE FILE NOT FOUND" << std::endl;
      assert(0);
    }
    cmd_fmtstr = "wget -qO- -o /dev/null %2$s | %1$s -dc";
  } else {
    std::ifstream testfile(trace_string);
    if (!testfile.good()) {
      std::cerr << "TRACE FILE NOT FOUND" << std::endl;
      assert(0);
    }
    cmd_fmtstr = "%1$s -dc %2$s";
  }

  if (last_dot[1] == 'g') // gzip format
    decomp_program = "gzip";
  else if (last_dot[1] == 'x') // xz
    decomp_program = "xz";
  else {
    std::cout << "ChampSim does not support traces other than gz or xz compression!" << std::endl;
    assert(0);
  }

#ifdef MULTIPLE_PAGE_SIZE
	std::cout << "trace ext file: " << trace_ext_string << std::endl;
  std::ifstream test_ext_file(trace_ext_string);
  if (!test_ext_file.good()) {
		std::cerr << "TRACE EXT FILE NOT FOUND" << std::endl;
		assert(0);
  }

  open(trace_string, trace_ext_string);
#else
  open(trace_string);
#endif
}

tracereader::~tracereader() { close(); }

template <typename T>
ooo_model_instr tracereader::read_single_instr()
{
#ifdef MULTIPLE_PAGE_SIZE
	T trace_read_instr;
	page_size_info ext_read_info;
	
	while (!fread(&trace_read_instr, sizeof(T), 1, trace_file) || !fread(&ext_read_info, sizeof(page_size_info), 1, trace_ext_file)) {
		// reached end of file for this trace extension
		std::cout << "*** Reached end of trace: " << trace_string << std::endl;
		std::cout << "*** Reached end of trace ext: " << trace_ext_string << std::endl;
		close();
		open(trace_string, trace_ext_string);
	}

	// copy the instruction into the performance model's instruction format
  ooo_model_instr retval(cpu, trace_read_instr, ext_read_info);
#else
  T trace_read_instr;

  while (!fread(&trace_read_instr, sizeof(T), 1, trace_file)) {
    // reached end of file for this trace
    std::cout << "*** Reached end of trace: " << trace_string << std::endl;

    // close the trace file and re-open it
    close();
    open(trace_string);
  }

  // copy the instruction into the performance model's instruction format
  ooo_model_instr retval(cpu, trace_read_instr);
#endif
  return retval;
}

#ifdef MULTIPLE_PAGE_SIZE
void tracereader::open(std::string trace_string, std::string trace_ext_string)
#else
void tracereader::open(std::string trace_string)
#endif
{
  char gunzip_command[4096];
  sprintf(gunzip_command, cmd_fmtstr.c_str(), decomp_program.c_str(), trace_string.c_str());
  trace_file = popen(gunzip_command, "r");
  if (trace_file == NULL) {
    std::cerr << std::endl << "*** CANNOT OPEN TRACE FILE: " << trace_string << " ***" << std::endl;
    assert(0);
  }
#ifdef MULTIPLE_PAGE_SIZE
	sprintf(gunzip_command, cmd_fmtstr.c_str(), decomp_program.c_str(), trace_ext_string.c_str());
	trace_ext_file = popen(gunzip_command, "r");
	if (trace_ext_file == NULL) {
		std::cerr << std::endl << "*** CANNOT OPEN TRACE EXTENSIONS FILE: " << trace_ext_string << " ***" << std::endl;
		assert(0);
	}
#endif
}

void tracereader::close()
{
  if (trace_file != NULL) {
    pclose(trace_file);
  }
#ifdef MULTIPLE_PAGE_SIZE
	if (trace_ext_file != NULL) {
		pclose(trace_ext_file);
	}	
#endif
}

class cloudsuite_tracereader : public tracereader
{
  ooo_model_instr last_instr;
  bool initialized = false;

public:
#ifdef MULTIPLE_PAGE_SIZE
  cloudsuite_tracereader(uint8_t cpu, std::string _tn, std::string _txn) : tracereader(cpu, _tn, _txn) {}
#else
	cloudsuite_tracereader(uint8_t cpu, std::string _tn) : tracereader(cpu, _tn) {}
#endif

  ooo_model_instr get()
  {
    ooo_model_instr trace_read_instr = read_single_instr<cloudsuite_instr>();

    if (!initialized) {
      last_instr = trace_read_instr;
      initialized = true;
    }

    last_instr.branch_target = trace_read_instr.ip;
    ooo_model_instr retval = last_instr;

    last_instr = trace_read_instr;
    return retval;
  }
};

class input_tracereader : public tracereader
{
  ooo_model_instr last_instr;
  bool initialized = false;

public:
#ifdef MULTIPLE_PAGE_SIZE
  input_tracereader(uint8_t cpu, std::string _tn, std::string _txn) : tracereader(cpu, _tn, _txn) {}
#else
	input_tracereader(uint8_t cpu, std::string _tn) : tracereader(cpu, _tn) {}
#endif

  ooo_model_instr get()
  {
    ooo_model_instr trace_read_instr = read_single_instr<input_instr>();

    if (!initialized) {
      last_instr = trace_read_instr;
      initialized = true;
    }

    last_instr.branch_target = trace_read_instr.ip;
    ooo_model_instr retval = last_instr;

    last_instr = trace_read_instr;
    return retval;
  }
};

#ifdef MULTIPLE_PAGE_SIZE
tracereader* get_tracereader(std::string fname, std::string fxname, uint8_t cpu, bool is_cloudsuite)
{
  if (is_cloudsuite) {
		std::cout << "Error: Cloudsuite not supported with multiple page sizes!" << std::endl;
		assert(false);
    //return new cloudsuite_tracereader(cpu, fname, fxname);
  } else {
    return new input_tracereader(cpu, fname, fxname);
  }
}
#else
tracereader* get_tracereader(std::string fname, uint8_t cpu, bool is_cloudsuite)
{
  if (is_cloudsuite) {
    return new cloudsuite_tracereader(cpu, fname);
  } else {
    return new input_tracereader(cpu, fname);
  }
}
#endif

