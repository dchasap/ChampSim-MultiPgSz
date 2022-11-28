
CPP = g++
CPPFLAGS = -std=c++11
LDFLAGS = -lboost_iostreams

trace_generator: trace_generator.o
		$(CPP) $(LDFLAGS) trace_generator.o -o trace_generator
trace_generator.o: trace_generator.cpp
		$(CPP) $(CPPFLAGS) -c trace_generator.cpp
install: trace_generator
		mv trace_generator ../bin/

clean:
	rm trace_generator.o trace_generator

