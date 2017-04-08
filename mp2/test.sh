#!/bin/bash
usage(){
        echo "usage: ./test.sh [--help | -h]
                 [--test-default | -t] [--test-gen] [--test-new]
                 [--test-custom <folder> <initcost_prefix> <topofilename>]
                 [--run-default | -r] [--run-gen] [--run-new]
                 [--run-custom <folder> <initcost_prefix> <topofilename>]
                 [--grade-default | -g] [--grade-gen] [--grade-new]
                 [--grade-custom <folder> <initcost_prefix> <topofilename> <gold_out>]
                 [--new-graph | -n] "
}

killall ls_router

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
        --test-gen)
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
        --test-mesg)
            sh ./scripts/run.sh
            sleep 5s
            ./manager_send $2 send $3 "________MESSAGE________"
            exit
            ;;
        --test-mesg-new)
            python ./scripts/generateTopology.py
            sh ./scripts/run.sh ./topology/ nodecosts networkTopology.txt
            sleep 8s
            ./manager_send $2 send $3 "________MESSAGE________"
            exit
            ;;
        --test-fb)
            sh ./scripts/run.sh
            sleep 5s
            ./killNodes.sh 1 2
            sleep 5s
            ./manager_send 6 send 3 "________FALL BACK________"
            exit
            ;;
        --test-fb-new)
            python ./scripts/generateTopology.py
            sh ./scripts/run.sh ./topology/ nodecosts networkTopology.txt
            sleep 5s
            ./manager_send 1 send 32 "________ORIGINAL_________"
            echo "."
            echo "."
            echo "."
            sleep 5s
            ./killNodes.sh 0
            sleep 5s
            ./manager_send 1 send 32 "________FALL BACK________"
            sleep 5s
            ./ls_router 0 ./topology/nodecosts0 log/log0 &
            sleep 5s
            ./manager_send 1 send 32 "________BETTER___________"
            exit
            ;;
        --run-default | -r)
            sh ./scripts/run.sh
            exit
            ;;
        --run-gen)
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
            sleep 5s
            sh ./scripts/grade.sh 0
            exit
            ;;
        --grade-gen) 
            sh ./scripts/run.sh ./topology/ nodecosts networkTopology.txt
            sleep 5s
            sh ./scripts/grade.sh 0
            exit
            ;;
        --grade-new)
            python ./scripts/generateTopology.py
            sh ./scripts/run.sh ./topology/ nodecosts networkTopology.txt
            sleep 5s
            sh ./scripts/grade.sh 0
            exit
            ;;
        --grade-custom)
            sh ./scripts/run.sh $2 $3 $4
            sleep 5s
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
