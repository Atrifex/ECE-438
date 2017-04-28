#!/bin/bash

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
