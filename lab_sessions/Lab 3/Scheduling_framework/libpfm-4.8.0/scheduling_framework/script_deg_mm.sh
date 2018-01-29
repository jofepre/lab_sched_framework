#!/bin/bash

tempfile1=$(mktemp)
tempfile2=$(mktemp)
tempfile3=$(mktemp)
sal=$(mktemp)
sal2=$(mktemp)

# This variables should be set to the number of nops
# that the microbenchmarks require to reach a bandwidth
# utilization by 100, 50, and 10 trans / usec.
nops_bw_100=
nops_bw_50=
nops_bw_10=



for nops in $nops_bw_100 $nops_bw_50 $nops_bw_10; do

    taskset -c 1 ./mm-microbenchmark $nops 0 > $tempfile1 &
    taskset -c 2 ./mm-microbenchmark $nops 0 > $tempfile2 &
    taskset -c 3 ./mm-microbenchmark $nops 0 > $tempfile3 &

    for bench in 2 3 5 8 9 13 14 15 21 24; do

	./scheduling_framework -W $bench -C 0 > $sal 2> $sal2
    
	cyc=$(cat sal2 | grep "Final_count" | awk '{print $5}')
	ins=$(cat sal2 | grep "Final_count" | awk '{print $7}')
	IPC=$(bc <<< "scale=3; $ins/$cyc")

    echo "Nops" $nops "Benchmark" $bench "IPC" $IPC
	echo "Nops" $nops "Benchmark" $bench "IPC" $IPC >> Results_deg_mem.res

    done

    killall -9 mm-microbenchmark
    
done


