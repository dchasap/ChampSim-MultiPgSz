#!/bin/bash

HOME=`pwd`
TRACE_DIR="/gpfs/projects/bsc18/romol/BranchPred/ChampSim/traces"
TRACE_EXT_DIR="/gpfs/projects/bsc18/romol/BranchPred/ChampSim/page_size_extensions"
RESULTS_DIR=${HOME}/results

BENCHSUITE=$1 
BIN=$2
DESCR_TAG=$3

source ${HOME}/scripts/benchmarks_all.sh

#export extra_opt="_idp_v${IDP_VICTIM_THRESHOLD}"

for trace in $TRACES; do

  export suffix=.champsimtrace.xz 
  export bench=${trace%$suffix}

echo "#!/bin/bash
#SBATCH -N 1
##SBATCH -n 1
##SBATCH -c 12
##SBATCH -c 48
#SBATCH -o ${HOME}/dump/${bench}${DESCR_TAG}_run.out 
#SBATCH -J chmpS_${bench}${DESCR_TAG}_run
#SBATCH --time=00:30:00 
##SBATCH --constraint=highmem
#SBATCH --qos=bsc_cs
##SBATCH --qos=debug

module load python/2.7.13

#export MISS_LATENCY_HIST_FILE=${HOME}/dump/${bench}${extra_opt}

#gdb -batch -ex "r" -ex "bt" -ex "q" --args ./bin/champsim --warmup_instructions 200000000 --simulation_instructions 500000000 ${TRACE_DIR}/$trace
#./bin/champsim --warmup_instructions 250000000 --simulation_instructions 1000000000 ${TRACE_DIR}/$trace ${TRACE_EXT_DIR}/${bench}.champsimtrace_ext_${contiguity}.xz
#./bin/champsim --warmup_instructions 250000000 --simulation_instructions 1000000000 ${TRACE_DIR}/$trace
./bin/${BIN} --warmup_instructions 50000000 --simulation_instructions 100000000 ${TRACE_DIR}/$trace
#./bin/${BIN} --warmup_instructions 5000000000 --simulation_instructions 1000000000 ${TRACE_DIR}/$trace
#./bin/${BIN} --warmup_instructions 2500000000 --simulation_instructions 500000000 ${TRACE_DIR}/$trace
#gdb -batch -ex "r" -ex "bt" -ex "q" --args ./bin/${BIN} --warmup_instructions 50000000 --simulation_instructions 100000000 ${TRACE_DIR}/$trace

" >	simr_${bench}_job.run
		sbatch simr_${bench}_job.run
		#chmod +x simt_${bench}_job.run
		#./simt_${bench}_job.run
		rm simr_${bench}_job.run
done

