#!/bin/bash

sudo tc qdisc del dev eth1 root 2>/dev/null

for i in $(seq 1 $1)
do
    echo "Testing iteration ${i}"
    echo "."
    echo "."
    echo "."
    ./reliable_sender 192.168.0.2 4950 sourcefile $2
    echo ""
done
