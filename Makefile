CC := gcc
CXX := g++
CFLAGS := -g -Wall -O3 -std=gnu99
CXXFLAGS := -g -Wall -O3 -std=c++17
CPPFLAGS :=  -Iinc -MMD -MP
LDFLAGS := 
LDLIBS := 

.phony: all clean

all: bin/champsim_xdip

clean: 
	$(RM) inc/champsim_constants.h
	$(RM) src/core_inst.cc
	$(RM) inc/cache_modules.inc
	$(RM) inc/ooo_cpu_modules.inc
	 find . -name \*.o -delete
	 find . -name \*.d -delete
	 $(RM) -r obj

	 find replacement/lru -name \*.o -delete
	 find replacement/lru -name \*.d -delete
	 find prefetcher/no -name \*.o -delete
	 find prefetcher/no -name \*.d -delete
	 find prefetcher/fnl_mma -name \*.o -delete
	 find prefetcher/fnl_mma -name \*.d -delete
	 find prefetcher/ipcp -name \*.o -delete
	 find prefetcher/ipcp -name \*.d -delete
	 find prefetcher/spp_dev -name \*.o -delete
	 find prefetcher/spp_dev -name \*.d -delete
	 find replacement/xdip -name \*.o -delete
	 find replacement/xdip -name \*.d -delete
	 find branch/hashed_perceptron -name \*.o -delete
	 find branch/hashed_perceptron -name \*.d -delete
	 find btb/basic_btb -name \*.o -delete
	 find btb/basic_btb -name \*.d -delete

bin/champsim_xdip: $(patsubst %.cc,%.o,$(wildcard src/*.cc)) obj/repl_rreplacementDlru.a obj/pref_pprefetcherDno.a obj/pref_pprefetcherDfnl_mma.a obj/pref_pprefetcherDipcp.a obj/pref_pprefetcherDspp_dev.a obj/repl_rreplacementDxdip.a obj/bpred_bbranchDhashed_perceptron.a obj/btb_bbtbDbasic_btb.a
	$(CXX) $(LDFLAGS) -o $@ $^ $(LDLIBS)

replacement/lru/%.o: CFLAGS += -Ireplacement/lru
replacement/lru/%.o: CXXFLAGS += -Ireplacement/lru
replacement/lru/%.o: CXXFLAGS +=  -Dinitialize_replacement=repl_rreplacementDlru_initialize -Dfind_victim=repl_rreplacementDlru_victim -Dupdate_replacement_state=repl_rreplacementDlru_update -Dreplacement_final_stats=repl_rreplacementDlru_final_stats
obj/repl_rreplacementDlru.a: $(patsubst %.cc,%.o,$(wildcard replacement/lru/*.cc)) $(patsubst %.c,%.o,$(wildcard replacement/lru/*.c))
	@mkdir -p $(dir $@)
	ar -rcs $@ $^

prefetcher/no/%.o: CFLAGS += -Iprefetcher/no
prefetcher/no/%.o: CXXFLAGS += -Iprefetcher/no
prefetcher/no/%.o: CXXFLAGS +=  -Dprefetcher_initialize=pref_pprefetcherDno_initialize -Dprefetcher_cache_operate=pref_pprefetcherDno_cache_operate -Dprefetcher_cache_fill=pref_pprefetcherDno_cache_fill -Dprefetcher_cycle_operate=pref_pprefetcherDno_cycle_operate -Dprefetcher_final_stats=pref_pprefetcherDno_final_stats -Dl1d_prefetcher_initialize=pref_pprefetcherDno_initialize -Dl2c_prefetcher_initialize=pref_pprefetcherDno_initialize -Dllc_prefetcher_initialize=pref_pprefetcherDno_initialize -Dl1d_prefetcher_operate=pref_pprefetcherDno_cache_operate -Dl2c_prefetcher_operate=pref_pprefetcherDno_cache_operate -Dllc_prefetcher_operate=pref_pprefetcherDno_cache_operate -Dl1d_prefetcher_cache_fill=pref_pprefetcherDno_cache_fill -Dl2c_prefetcher_cache_fill=pref_pprefetcherDno_cache_fill -Dllc_prefetcher_cache_fill=pref_pprefetcherDno_cache_fill -Dl1d_prefetcher_final_stats=pref_pprefetcherDno_final_stats -Dl2c_prefetcher_final_stats=pref_pprefetcherDno_final_stats -Dllc_prefetcher_final_stats=pref_pprefetcherDno_final_stats
obj/pref_pprefetcherDno.a: $(patsubst %.cc,%.o,$(wildcard prefetcher/no/*.cc)) $(patsubst %.c,%.o,$(wildcard prefetcher/no/*.c))
	@mkdir -p $(dir $@)
	ar -rcs $@ $^

prefetcher/fnl_mma/%.o: CFLAGS += -Iprefetcher/fnl_mma
prefetcher/fnl_mma/%.o: CXXFLAGS += -Iprefetcher/fnl_mma
prefetcher/fnl_mma/%.o: CXXFLAGS +=  -Dprefetcher_initialize=pref_pprefetcherDfnl_mma_initialize -Dprefetcher_branch_operate=pref_pprefetcherDfnl_mma_branch_operate -Dprefetcher_cache_operate=pref_pprefetcherDfnl_mma_cache_operate -Dprefetcher_cycle_operate=pref_pprefetcherDfnl_mma_cycle_operate -Dprefetcher_cache_fill=pref_pprefetcherDfnl_mma_cache_fill -Dprefetcher_final_stats=pref_pprefetcherDfnl_mma_final_stats -Dl1i_prefetcher_initialize=pref_pprefetcherDfnl_mma_initialize -Dl1i_prefetcher_branch_operate=pref_pprefetcherDfnl_mma_branch_operate -Dl1i_prefetcher_cache_operate=pref_pprefetcherDfnl_mma_cache_operate -Dl1i_prefetcher_cycle_operate=pref_pprefetcherDfnl_mma_cycle_operate -Dl1i_prefetcher_cache_fill=pref_pprefetcherDfnl_mma_cache_fill -Dl1i_prefetcher_final_stats=pref_pprefetcherDfnl_mma_final_stats
obj/pref_pprefetcherDfnl_mma.a: $(patsubst %.cc,%.o,$(wildcard prefetcher/fnl_mma/*.cc)) $(patsubst %.c,%.o,$(wildcard prefetcher/fnl_mma/*.c))
	@mkdir -p $(dir $@)
	ar -rcs $@ $^

prefetcher/ipcp/%.o: CFLAGS += -Iprefetcher/ipcp
prefetcher/ipcp/%.o: CXXFLAGS += -Iprefetcher/ipcp
prefetcher/ipcp/%.o: CXXFLAGS +=  -Dprefetcher_initialize=pref_pprefetcherDipcp_initialize -Dprefetcher_cache_operate=pref_pprefetcherDipcp_cache_operate -Dprefetcher_cache_fill=pref_pprefetcherDipcp_cache_fill -Dprefetcher_cycle_operate=pref_pprefetcherDipcp_cycle_operate -Dprefetcher_final_stats=pref_pprefetcherDipcp_final_stats -Dl1d_prefetcher_initialize=pref_pprefetcherDipcp_initialize -Dl2c_prefetcher_initialize=pref_pprefetcherDipcp_initialize -Dllc_prefetcher_initialize=pref_pprefetcherDipcp_initialize -Dl1d_prefetcher_operate=pref_pprefetcherDipcp_cache_operate -Dl2c_prefetcher_operate=pref_pprefetcherDipcp_cache_operate -Dllc_prefetcher_operate=pref_pprefetcherDipcp_cache_operate -Dl1d_prefetcher_cache_fill=pref_pprefetcherDipcp_cache_fill -Dl2c_prefetcher_cache_fill=pref_pprefetcherDipcp_cache_fill -Dllc_prefetcher_cache_fill=pref_pprefetcherDipcp_cache_fill -Dl1d_prefetcher_final_stats=pref_pprefetcherDipcp_final_stats -Dl2c_prefetcher_final_stats=pref_pprefetcherDipcp_final_stats -Dllc_prefetcher_final_stats=pref_pprefetcherDipcp_final_stats
obj/pref_pprefetcherDipcp.a: $(patsubst %.cc,%.o,$(wildcard prefetcher/ipcp/*.cc)) $(patsubst %.c,%.o,$(wildcard prefetcher/ipcp/*.c))
	@mkdir -p $(dir $@)
	ar -rcs $@ $^

prefetcher/spp_dev/%.o: CFLAGS += -Iprefetcher/spp_dev
prefetcher/spp_dev/%.o: CXXFLAGS += -Iprefetcher/spp_dev
prefetcher/spp_dev/%.o: CXXFLAGS +=  -Dprefetcher_initialize=pref_pprefetcherDspp_dev_initialize -Dprefetcher_cache_operate=pref_pprefetcherDspp_dev_cache_operate -Dprefetcher_cache_fill=pref_pprefetcherDspp_dev_cache_fill -Dprefetcher_cycle_operate=pref_pprefetcherDspp_dev_cycle_operate -Dprefetcher_final_stats=pref_pprefetcherDspp_dev_final_stats -Dl1d_prefetcher_initialize=pref_pprefetcherDspp_dev_initialize -Dl2c_prefetcher_initialize=pref_pprefetcherDspp_dev_initialize -Dllc_prefetcher_initialize=pref_pprefetcherDspp_dev_initialize -Dl1d_prefetcher_operate=pref_pprefetcherDspp_dev_cache_operate -Dl2c_prefetcher_operate=pref_pprefetcherDspp_dev_cache_operate -Dllc_prefetcher_operate=pref_pprefetcherDspp_dev_cache_operate -Dl1d_prefetcher_cache_fill=pref_pprefetcherDspp_dev_cache_fill -Dl2c_prefetcher_cache_fill=pref_pprefetcherDspp_dev_cache_fill -Dllc_prefetcher_cache_fill=pref_pprefetcherDspp_dev_cache_fill -Dl1d_prefetcher_final_stats=pref_pprefetcherDspp_dev_final_stats -Dl2c_prefetcher_final_stats=pref_pprefetcherDspp_dev_final_stats -Dllc_prefetcher_final_stats=pref_pprefetcherDspp_dev_final_stats
obj/pref_pprefetcherDspp_dev.a: $(patsubst %.cc,%.o,$(wildcard prefetcher/spp_dev/*.cc)) $(patsubst %.c,%.o,$(wildcard prefetcher/spp_dev/*.c))
	@mkdir -p $(dir $@)
	ar -rcs $@ $^

replacement/xdip/%.o: CFLAGS += -Ireplacement/xdip
replacement/xdip/%.o: CXXFLAGS += -Ireplacement/xdip
replacement/xdip/%.o: CXXFLAGS +=  -Dinitialize_replacement=repl_rreplacementDxdip_initialize -Dfind_victim=repl_rreplacementDxdip_victim -Dupdate_replacement_state=repl_rreplacementDxdip_update -Dreplacement_final_stats=repl_rreplacementDxdip_final_stats
obj/repl_rreplacementDxdip.a: $(patsubst %.cc,%.o,$(wildcard replacement/xdip/*.cc)) $(patsubst %.c,%.o,$(wildcard replacement/xdip/*.c))
	@mkdir -p $(dir $@)
	ar -rcs $@ $^

branch/hashed_perceptron/%.o: CFLAGS += -Ibranch/hashed_perceptron
branch/hashed_perceptron/%.o: CXXFLAGS += -Ibranch/hashed_perceptron
branch/hashed_perceptron/%.o: CXXFLAGS +=  -Dinitialize_branch_predictor=bpred_bbranchDhashed_perceptron_initialize -Dlast_branch_result=bpred_bbranchDhashed_perceptron_last_result -Dpredict_branch=bpred_bbranchDhashed_perceptron_predict
obj/bpred_bbranchDhashed_perceptron.a: $(patsubst %.cc,%.o,$(wildcard branch/hashed_perceptron/*.cc)) $(patsubst %.c,%.o,$(wildcard branch/hashed_perceptron/*.c))
	@mkdir -p $(dir $@)
	ar -rcs $@ $^

btb/basic_btb/%.o: CFLAGS += -Ibtb/basic_btb
btb/basic_btb/%.o: CXXFLAGS += -Ibtb/basic_btb
btb/basic_btb/%.o: CXXFLAGS +=  -Dinitialize_btb=btb_bbtbDbasic_btb_initialize -Dupdate_btb=btb_bbtbDbasic_btb_update -Dbtb_prediction=btb_bbtbDbasic_btb_predict
obj/btb_bbtbDbasic_btb.a: $(patsubst %.cc,%.o,$(wildcard btb/basic_btb/*.cc)) $(patsubst %.c,%.o,$(wildcard btb/basic_btb/*.c))
	@mkdir -p $(dir $@)
	ar -rcs $@ $^

-include $(wildcard src/*.d)
-include $(wildcard replacement/lru/*.d)
-include $(wildcard prefetcher/no/*.d)
-include $(wildcard prefetcher/fnl_mma/*.d)
-include $(wildcard prefetcher/ipcp/*.d)
-include $(wildcard prefetcher/spp_dev/*.d)
-include $(wildcard replacement/xdip/*.d)
-include $(wildcard branch/hashed_perceptron/*.d)
-include $(wildcard btb/basic_btb/*.d)

