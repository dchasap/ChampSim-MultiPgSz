
TRACES="
403.gcc-16B.champsimtrace.xz
403.gcc-17B.champsimtrace.xz  
403.gcc-48B.champsimtrace.xz   
450.soplex-92B.champsimtrace.xz
450.soplex-247B.champsimtrace.xz
471.omnetpp-188B.champsimtrace.xz
473.astar-42B.champsimtrace.xz          
473.astar-153B.champsimtrace.xz  
473.astar-359B.champsimtrace.xz
compute_int_906.champsimtrace.xz
"
TRACES="
403.gcc-16B.champsimtrace.xz
403.gcc-17B.champsimtrace.xz  
403.gcc-48B.champsimtrace.xz   
450.soplex-92B.champsimtrace.xz
450.soplex-247B.champsimtrace.xz
471.omnetpp-188B.champsimtrace.xz
473.astar-42B.champsimtrace.xz          
473.astar-153B.champsimtrace.xz  
473.astar-359B.champsimtrace.xz
"

BENCHMARKS="
403.gcc 
450.soplex 
471.omnetpp 
473.astar 
compute_int_906
"
BENCHMARKS="
403.gcc 
450.soplex 
471.omnetpp 
473.astar 
"

for trace in $TRACES; do
	export suffix=.champsimtrace.xz 
	export bench=${trace%$suffix}
	#echo $bench
	SIMPOINTS="$SIMPOINTS $bench"
done

