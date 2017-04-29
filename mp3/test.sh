#!/bin/bash
usage(){
        echo "usage: ./test.sh [--help | -h]
        [--generate-random | -r]
        [--generate-seq | -s]
        [--test-perfect-s | --tps]"

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
            sh ./scripts/testPefectSender.sh $2
            exit
            ;;
        --test-perfect-r | --tpr)
            sh ./scripts/testPefectReceiver.sh $2
            exit
            ;;
        --test-lossy-s | --tls)
            sh ./scripts/testLossySender.sh $2 $3 $4
            exit
            ;;
        --test-lossy-r | --tlr)
            sh ./scripts/testLossyReceiver.sh $2
            exit
            ;;
    esac
    shift
done

usage
