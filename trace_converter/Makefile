
CPP = g++
#CPPFLAGS = -g -std=c++11
CPPFLAGS = -std=c++11 -O3
LDFLAGS = -lboost_iostreams

trace_converter: trace_converter.o
		$(CPP) $(LDFLAGS) trace_converter.o -o trace_converter
trace_converter.o: trace_converter.cpp
		$(CPP) $(CPPFLAGS) -c trace_converter.cpp
install: trace_converter
		mv trace_converter ../bin/

clean:
		rm trace_converter.o trace_converter

