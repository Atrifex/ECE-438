#!/bin/bash
usage(){
        echo "usage: ./test.sh [--help | -h]
                 [--test-default | -t] [--test-generated] [--test-new]
                 [--test-custom <folder> <initcost_prefix> <topofilename>]
                 [--run-default | -r] [--run-generated] [--run-new]
                 [--run-custom <folder> <initcost_prefix> <topofilename>]
                 [--grade-default | -g] [--grade-generated] [--grade-new]
                 [--grade-custom <folder> <initcost_prefix> <topofilename> <gold_out>]
                 [--new-graph | -n] "
}

while [ ! $# -eq 0 ]
do
    case "$1" in
        --help | -h)
            usage
            exit
            ;;
        --test-default | -t)
            python ./scripts/topotest.py ./example_topology test2initcosts topoexample.txt
            exit
            ;;
        --test-generated)
            python ./scripts/topotest.py ./topology nodecosts networkTopology.txt
            exit
            ;;
        --test-new)
            python ./scripts/generateTopology.py
            python ./scripts/topotest.py ./topology nodecosts networkTopology.txt
            exit
            ;;
        --test-custom)
            python ./scripts/topotest.py $2 $3 $4
            exit
            shift; shift; shift;
            ;;
        --run-default | -r)
            sh ./scripts/run.sh
            exit
            ;;
        --run-generated)
            sh ./scripts/run.sh ./topology/ nodecosts networkTopology.txt
            exit
            ;;
        --run-new)
            python ./scripts/generateTopology.py
            sh ./scripts/run.sh ./topology/ nodecosts networkTopology.txt
            exit
            ;;
        --run-custom)
            sh ./scripts/run.sh $2 $3 $4
            exit
            shift; shift; shift;
            ;;
        --grade-default | -g)
            sh ./scripts/run.sh
            sleep 7s
            sh ./scripts/grade.sh
            exit
            ;;
        --grade-generated) 
            sh ./scripts/run.sh ./topology/ nodecosts networkTopology.txt
            sleep 7s
            sh ./scripts/grade.sh ./topology/goldNetwork.txt
            exit
            ;;
        --grade-new)
            python ./scripts/generateTopology.py
            sh ./scripts/run.sh ./topology/ nodecosts networkTopology.txt
            sleep 7s
            sh ./scripts/grade.sh ./topology/goldNetwork.txt
            exit
            ;;
        --grade-custom)
            sh ./scripts/run.sh $2 $3 $4
            sleep 7s
            sh ./scripts/grade.sh $5
            exit
            shift; shift; shift; shift;
            ;;
        --new-graph | -n)
            python ./scripts/generateTopology.py
            exit
            ;;
    esac
    shift
done

usage
