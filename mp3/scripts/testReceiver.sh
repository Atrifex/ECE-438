#!/bin/bash

for i in $(seq 1 $1)
do
    rm -f destfile
    echo "Testing iteration ${i}"
    echo "."
    echo "."
    echo "."
    ./reliable_receiver 4950 destfile
    if ! diff -q "sourcefile" "destfile"; then
        echo "Test iteration ${i} FAILED! FIX BUGS!"
        exit;
    fi
    echo ""
done

echo "ALL TEST ITERATIONS PASSED! GOOD JOB!"
