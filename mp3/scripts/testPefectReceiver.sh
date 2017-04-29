#!/bin/bash

sudo tc qdisc del dev eth1 root 2>/dev/null


for i in $(seq 1 $1)
do
    echo "Testing iteration ${i}"
    echo "."
    echo "."
    echo "."
    ./reliable_receiver 4950 destfile
    diff destfile sourcefile
    echo ""
done
