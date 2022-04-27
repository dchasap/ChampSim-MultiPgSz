#!/bin/bash

HOME="/gpfs/home/bsc18/bsc18186/scratch/tlb_multipage_replacement/ChampSim"
TRACE_DIR="/gpfs/projects/bsc18/romol/BranchPred/ChampSim/traces"
TRACE_EXT_DIR="/gpfs/projects/bsc18/romol/BranchPred/ChampSim/page_size_extensions"
RESULTS_DIR=${HOME}/results

source ${HOME}/scripts/benchmarks.sh

TRACES="403.gcc-17B.champsimtrace.xz"

export extra_opt="_test_new"
export lru_dist="4"
export contiguity="550"

for trace in $TRACES; do

  export suffix=.champsimtrace.xz 
  export bench=${trace%$suffix}

	mv ${TRACE_EXT_DIR}/${bench}.champsimtrace_ext_6050.xz ${TRACE_EXT_DIR}/${bench}.champsimtrace_ext_${contiguity}.xz

echo "#!/bin/bash
#SBATCH -N 1
##SBATCH -n 1
##SBATCH -c 12
##SBATCH -c 48
#SBATCH -o ${HOME}/dump/${bench}${extra_opt}_run.out 
#SBATCH -J chmpS_${bench}${extra_opt}_run
##SBATCH --time=6:00:00 
##SBATCH --constraint=highmem
##SBATCH --qos=bsc_cs
#SBATCH --qos=debug

module load python/2.7.13

export LRU_DISTANCE=${lru_dist}
#export REUSE_DIST_CSV=${RESULTS_DIR}/${bench}_pow.csv
#export ENABLE_TRACE_RUN=False
#export MEMORY_ACCESS_TRACE_FILE=${HOME}/mem_traces/${bench}.ma

#gdb -batch -ex "r" -ex "bt" -ex "q" --args ./bin/champsim --warmup_instructions 200000000 --simulation_instructions 500000000 ${TRACE_DIR}/$trace
#./bin/champsim --warmup_instructions 250000000 --simulation_instructions 1000000000 ${TRACE_DIR}/$trace ${TRACE_EXT_DIR}/${bench}.champsimtrace_ext_${contiguity}.xz
./bin/champsim --warmup_instructions 250000000 --simulation_instructions 1000000000 ${TRACE_DIR}/$trace ${TRACE_EXT_DIR}/${bench}.champsimtrace_ext_${contiguity}.xz

" >	simr_${bench}_job.run
		sbatch simr_${bench}_job.run
		#chmod +x simt_${bench}_job.run
		#./simt_${bench}_job.run
		rm simr_${bench}_job.run
done

