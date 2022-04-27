#include <cstdio>
#include <string>

#include "instruction.h"

class tracereader
{
protected:
  FILE* trace_file = NULL;
  uint8_t cpu;
  std::string cmd_fmtstr;
  std::string decomp_program;
  std::string trace_string;
#ifdef MULTIPLE_PAGE_SIZE
	FILE* trace_ext_file = NULL;
	std::string trace_ext_string;
#endif

public:
  tracereader(const tracereader& other) = delete;
#ifdef MULTIPLE_PAGE_SIZE
  tracereader(uint8_t cpu, std::string _ts, std::string _txs);
#else
	tracereader(uint8_t cpu, std::string _ts);
#endif
	~tracereader();
#ifdef MULTIPLE_PAGE_SIZE
  void open(std::string trace_string, std::string trace_ext_string);
#else
  void open(std::string trace_string);
#endif
  void close();

  template <typename T>
  ooo_model_instr read_single_instr();

  virtual ooo_model_instr get() = 0;
};

#ifdef MULTIPLE_PAGE_SIZE
tracereader* get_tracereader(std::string fname, std::string fxname, uint8_t cpu, bool is_cloudsuite);
#else
tracereader* get_tracereader(std::string fname, uint8_t cpu, bool is_cloudsuite);
#endif

