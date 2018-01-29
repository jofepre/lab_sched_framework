#!/bin/bash

EVENTS="llc_misses llc_references"

###########################
tempfile=$(mktemp)
TIMESTAMP=$(date +%F_%T)
mkdir -p results/$TIMESTAMP

echo FOLDER: results/$TIMESTAMP
echo benchmark cycles instructions $EVENTS

for bench in 0 1 3; do

	./scheduling_framework -W $bench -p -e $(echo $EVENTS | tr " " , ) >/dev/null 2>$tempfile

    cyc=$(cat $tempfile | grep "Final_count" | awk '{print $5}')
	ins=$(cat $tempfile | grep "Final_count" | awk '{print $7}')
	ev0=$(cat $tempfile | grep "Final_count" | awk '{print $9}')
	ev1=$(cat $tempfile | grep "Final_count" | awk '{print $11}')
	ev2=$(cat $tempfile | grep "Final_count" | awk '{print $13}')
	ev3=$(cat $tempfile | grep "Final_count" | awk '{print $15}')
	ev4=$(cat $tempfile | grep "Final_count" | awk '{print $17}')
	ev5=$(cat $tempfile | grep "Final_count" | awk '{print $19}')

	echo $bench $cyc $ins $ev0 $ev1 $ev2 $ev3 $ev4 $ev5 
        
    RESULTS_FILE=results/$TIMESTAMP/$bench.res
    echo cycles instructions $EVENTS >$RESULTS_FILE         # legend
    cat $tempfile | grep "Quantum_count" | awk '{print $5 " " $6 " " $7 " " $8 " " $9 " " $10 " " $11 " " $12}' >>$RESULTS_FILE
done
exit
