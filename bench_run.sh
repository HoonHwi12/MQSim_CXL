#!/bin/bash

BASE_DIR="/home/hoonhwi/MQSIM"
BENCHMARKS_DIR="$BASE_DIR/benchmarks/scm_2year"
TRACE_GEN_DIR="$BASE_DIR/trace_generator/run_script"
MQSIM_DIR="$BASE_DIR/MQSim_CXL_Linux"
TRACE_TRANS_DIR="$BASE_DIR/trace_translation"
RESULT_DIR="$TRACE_GEN_DIR/result"
WORKLOAD_DIR="$MQSIM_DIR/workload_xmls"

BENCHMARKS=("alexnet" "redis" "ycsb" "opt" "lstm" "gapbs" "vecadd" "squeezenet" "googlenet")
#BENCHMARKS=("alexnet" "redis" "ycsb" "opt" "lstm" "gapbs" "vecadd")
#BENCHMARKS=("squeezenet" "googlenet")

echo "make clean && make -j"
make clean && make -j

for INSTR_MAPPING in 0; do
    for BENCH in "${BENCHMARKS[@]}"; do
        for CXL_BUFFER_KB in 0; do
            echo ===================== ${BENCH^^} =====================
            
            # cd $TRACE_GEN_DIR

            # echo "Run Trace Generator"
            # sudo ./run_script.sh --type physical --input $BENCHMARKS_DIR/$BENCH

            # cd $MQSIM_DIR

            # echo "Move Trace and Plot"
            # sudo mv $TRACE_GEN_DIR/trace_prefOFF.vout $RESULT_DIR/vout_bench/$BENCH.vout
            # sudo mv $TRACE_GEN_DIR/trace_prefOFF.pout $RESULT_DIR/pout_bench/$BENCH.pout
            # sudo mv $TRACE_GEN_DIR/trace_prefOFF.slog $RESULT_DIR/$BENCH.slog
            # sudo mv $TRACE_GEN_DIR/trace_prefOFF.vpmap $RESULT_DIR/$BENCH.vpmap
            # sudo mv $TRACE_GEN_DIR/plot-*va* $RESULT_DIR/trace_plot/${BENCH}_va.png
            # sudo mv $TRACE_GEN_DIR/plot-*pa* $RESULT_DIR/trace_plot/${BENCH}_pa.png
            # sudo mv $TRACE_GEN_DIR/proclog.log $RESULT_DIR/${BENCH}_proc.log
            # sudo mv $TRACE_GEN_DIR/callgrind* $RESULT_DIR/${BENCH}_callgrind.out

            # echo "Copy Result pout File to Trace Translator"
            # cp $RESULT_DIR/pout_bench/$BENCH.pout $TRACE_TRANS_DIR

            # echo "Run Trace Translator"
            # $TRACE_TRANS_DIR/Source_arg $TRACE_TRANS_DIR/$BENCH.pout

            echo "./MQSim -i ./ssdconfig.xml -w $WORKLOAD_DIR/workload_$BENCH.xml -c ${CXL_BUFFER_KB} -s ${INSTR_MAPPING}"
            ./MQSim -i ./ssdconfig.xml -w $WORKLOAD_DIR/workload_$BENCH.xml -c ${CXL_BUFFER_KB} -s ${INSTR_MAPPING}

            mkdir -p ./Results/INSTR${INSTR_MAPPING}/MEM${CXL_BUFFER_KB}KB
            mkdir -p ./Results/INSTR${INSTR_MAPPING}/MEM${CXL_BUFFER_KB}KB/${BENCH^^}

            echo "Move results to the benchmark-specific directory"
            mv ./Results/*.txt ./Results/INSTR${INSTR_MAPPING}/MEM${CXL_BUFFER_KB}KB/${BENCH^^}/
            mv ${WORKLOAD_DIR}/workload_${BENCH}_scenario_1.xml ./Results/INSTR${INSTR_MAPPING}/MEM${CXL_BUFFER_KB}KB/${BENCH^^}/
        done
    done
done
