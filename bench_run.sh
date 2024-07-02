#!/bin/bash

BASE_DIR="/home/hoonhwi/MQSim_CXL_Linux"
TRACE_GEN_DIR="$BASE_DIR/trace_generator/run_script"
MQSIM_DIR="$BASE_DIR/MQSim_CXL_Linux"
TRACE_TRANS_DIR="$BASE_DIR/trace_translation"
RESULT_DIR="$TRACE_GEN_DIR/result"
WORKLOAD_DIR="$MQSIM_DIR/workload_xmls"
BENCHMARKS_DIR="/home/junwoo/benchmarks/scm_2year"

BENCHMARKS=("alexnet" "bench_alexnet" "bench_googlenet" "bench_lstm" "bench_squeezenet" "bertbase16" "convolution" "dlrmlarge" "googlenet" "lstm" "matmul" "opt13b164" "random" "squeezenet" "vecadd")

for BENCH in "${BENCHMARKS[@]}"; do
    echo ===================== ${BENCH^^} =====================

    # Change Directory for Trace Generator
    cd $TRACE_GEN_DIR

    # Run Trace Generator
    sudo ./run_script.sh --type physical --input $BENCHMARKS_DIR/$BENCH

    # Come Back to MQSim Directory
    cd $MQSIM_DIR

    # Move Trace and Plot
    sudo mv $TRACE_GEN_DIR/trace_prefOFF.vout $RESULT_DIR/vout_bench/$BENCH.vout
    sudo mv $TRACE_GEN_DIR/trace_prefOFF.pout $RESULT_DIR/pout_bench/$BENCH.pout
    sudo mv $TRACE_GEN_DIR/trace_prefOFF.slog $RESULT_DIR/$BENCH.slog
    sudo mv $TRACE_GEN_DIR/trace_prefOFF.vpmap $RESULT_DIR/$BENCH.vpmap
    sudo mv $TRACE_GEN_DIR/plot-*va* $RESULT_DIR/trace_plot/${BENCH}_va.png
    sudo mv $TRACE_GEN_DIR/plot-*pa* $RESULT_DIR/trace_plot/${BENCH}_pa.png
    sudo mv $TRACE_GEN_DIR/proclog.log $RESULT_DIR/${BENCH}_proc.log
    sudo mv $TRACE_GEN_DIR/callgrind* $RESULT_DIR/${BENCH}_callgrind.out

    # Copy Result pout File to Trace Translator
    cp $RESULT_DIR/pout_bench/$BENCH.pout $TRACE_TRANS_DIR

    # Run Trace Translator
    $TRACE_TRANS_DIR/Source_arg $TRACE_TRANS_DIR/$BENCH.pout

    # Run MQSim_Linux
    ./MQSim -i ./ssdconfig.xml -w $WORKLOAD_DIR/workload_$BENCH.xml

    # Create results directory if it doesn't exist
    mkdir -p ./Results/${BENCH^^}

    # Move results to the benchmark-specific directory
    mv ./Results/*.txt ./Results/${BENCH^^}
done
