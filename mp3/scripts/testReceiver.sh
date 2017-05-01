#!/bin/bash

timestamp() {
     date +"%T"
}

for i in $(seq 1 $1)
do
    # rm -f destfile
    timestamp
    echo "Testing iteration ${i}"
    ./reliable_receiver 4950 destfile
    if ! diff -q "sourcefile" "destfile"; then
        diff sourcefile destfile > errors
        echo "Test iteration ${i} FAILED! FIX BUGS!"
        exit;
    fi
    timestamp
    echo ""
done

echo "ALL TEST ITERATIONS PASSED! GOOD JOB!"
