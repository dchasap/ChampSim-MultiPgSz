
source ./scripts/spec_cpu_workloads.sh
source ./scripts/gap_workloads.sh
source ./scripts/qualcom_srv_workloads.sh

#TRACES="${SPEC_CPU_2006} ${SPEC_CPU_2017}"
#TRACES="${GAP}"
#TRACES=${QUALCOM_SRV1}
#TRACES="srv_0.champsimtrace.xz"
#TRACES="${SELECTED_QUALCOM_SRV}"

if [ "${BENCHSUITE}" == "qualcom" ];then
	    TRACES="${QUALCOM_SRV1} ${QUALCOM_SRV2}"
elif [ "${BENCHSUITE}" == "selected_qualcom"  ]; then
			TRACES="${SELECTED_QUALCOM_SRV}"
elif [ "${BENCHSUITE}" == "gap" ]; then
			TRACES="${GAP}"
elif [ "${BENCHSUITE}" == "test" ]; then 
			TRACES="srv_44.champsimtrace.xz"
fi

for trace in $TRACES; do
	export suffix=.champsimtrace.xz 
	export bench=${trace%$suffix}
	#echo $bench
	SIMPOINTS="$SIMPOINTS $bench"
done

for simpoint in $SIMPOINTS; do
	export suffix=-*
	export bench=${simpoint%$suffix}
	BENCHMARKS="${BENCHMARKS} ${bench}"
done
BENCHMARKS=$(echo "${BENCHMARKS}" | xargs -n1 | sort -u | xargs)

