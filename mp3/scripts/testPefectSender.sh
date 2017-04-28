#!/bin/bash

for i in $(seq 1 $1)
do
    echo "Testing iteration ${i}"
    echo "."
    echo "."
    echo "."
    ./reliable_sender 192.168.0.2 4950 sourcefile 13123123123123
    diff destfile sourcefile
    echo ""
done
