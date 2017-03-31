#!/bin/bash
usage(){
        echo "usage: ./test.sh [--help | -h]
                 [--test-default | -t] [--test-custom] [--test-new]
                 [--run-default | -t] [--run-custom] [--run-new]
                 [--grade-default | -t] [--grade-custom] [--grade-new]
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
        --test-custom)
            python ./scripts/topotest.py ./topology nodecosts networkTopology.txt
            exit
            ;;
        --test-new)
            python ./scripts/createTopology.py
            python ./scripts/topotest.py ./topology nodecosts networkTopology.txt
            exit
            ;;
        --run-default | -r)
            sh ./scripts/run.sh
            exit
            ;;
        --run-custom)
            sh ./scripts/run.sh ./topology nodecosts networkTopology.txt
            exit
            ;;
        --run-new)
            python ./scripts/createTopology.py
            sh ./scripts/run.sh ./topology nodecosts networkTopology.txt
            exit
            ;;
        --grade-default | -g)
            sh ./scripts/run.sh
            sleep 7s
            sh ./scripts/grade.sh
            exit
            ;;
        --grade-custom) 
            sh ./scripts/run.sh ./topology nodecosts networkTopology.txt
            sleep 7s
            sh ./scripts/grade.sh
            exit
            ;;
        --grade-new)
            python ./scripts/createTopology.py
            sh ./scripts/run.sh ./topology nodecosts networkTopology.txt
            sleep 7s
            sh ./scripts/grade.sh # TODO: Change inputs
            exit
            ;;
        --new-graph | -n)
            python ./scripts/createTopology.py
            exit
            ;;
    esac
    shift
done

usage
