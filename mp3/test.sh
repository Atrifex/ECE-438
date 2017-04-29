#!/bin/bash
usage(){
        echo "usage: open script and look at options. Too many to document here."
}

make

while [ ! $# -eq 0 ]
do
    case "$1" in
        --generate-random | -r)
            sh ./scripts/randomFileGenerator.sh
            exit
            ;;
        --generate-seq | -s)
            sh ./scripts/sequentialNumbers.sh $2 $3
            exit
            ;;
        --test-perfect-s | --tps)
            sh ./scripts/testPefectSender.sh $2 $3
            exit
            ;;
        --test-lossy-s | --tls)
            sh ./scripts/testLossySender.sh $2 $3 $4 $5
            exit
            ;;
        --test-r | --tr)
            sh ./scripts/testReceiver.sh $2
            exit
            ;;
    esac
    shift
done

usage
