class CMP_TRACE{
	public:
		FILE * trace_file;
		char trace_string[1024];
		char gunzip_command[1024];
	
	CMP_TRACE(){
		trace_file = NULL;
	}
};


class TRACE_Extension {
	public:
		FILE * extension_file;
		char extension_string[1024];
		char compress_command[1024];
};

