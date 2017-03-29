#!/bin/bash

# Settings
DEFAULT_TOPO_DIR="./example_topology/"
DEFAULT_TOPO_FILE_PREFIX="test2initcosts"
DEFAULT_TOPO_FILE="topoexample.txt"

# Command line parameters --> will be same as default for debug purposes
TOPO_DIR=${DEFAULT_TOPO_DIR}
TOPO_FILE_PREFIX=${DEFAULT_TOPO_FILE_PREFIX}
TOPO_FILE=${DEFAULT_TOPO_FILE}
INIT_COSTS=$(find ${TOPO_DIR} -maxdepth 1 -name "${TOPO_FILE_PREFIX}*" -print)

DEBUG_NODE=-1

while getopts ":d:n:a:" opt; do
  case $opt in
    n)
        DEBUG_NODE=${OPTARG}
        ;;
    \?)
        echo "Invalid option: -$OPTARG" >&2
        exit 1
        ;;
    :)
        echo "Option -$OPTARG requires an argument." >&2
        exit 1
        ;;
  esac
done

pkill ls_router
make clean
make

rm -rf ./logfiles/
mkdir ./logfiles/

perl make_topology.pl ${TOPO_DIR}${TOPO_FILE}

echo "Using graphs from ${TOPO_DIR}"

for FILE_NAME in ${INIT_COSTS}; do
    NODE_ID=${FILE_NAME#*${TOPO_FILE_PREFIX}}
    if [ "${NODE_ID}" -eq "${DEBUG_NODE}" ]; then
        continue
    fi
    ./ls_router ${NODE_ID} ${FILE_NAME} ./logfiles/log${NODE_ID} &
done


if [ "${DEBUG_NODE}" -ne "-1" ]; then
    gdb ./ls_router ${DEBUG_NODE} ./example_topology/test2initcosts255 ./logfiles/log255
fi
