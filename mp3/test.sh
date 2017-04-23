#!/bin/bash
usage(){
        echo "usage: ./test.sh [--help | -h]
        [--generate-random | -r]
        [--generate-seq | -s]"
                 
}

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
    esac
    shift
done

usage
