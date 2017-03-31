#!/bin/bash

# Settings
DEFAULT_TOPO_DIR="./logfiles/"
DEFAULT_TOPO_FILE_PREFIX="graph"

# Command line parameters
TOPO_DIR=${DEFAULT_TOPO_DIR}
TOPO_FILE_PREFIX=${DEFAULT_TOPO_FILE_PREFIX}
GRAPH_DISP=$(find ${TOPO_DIR} -maxdepth 1 -name "${TOPO_FILE_PREFIX}*" -print)

FILE_END=${1:--1}

if [ ${FILE_END} -eq -1 ]; then
    echo "Specify node to compare to" 
    exit 1
fi

pkill ls_router
sudo iptables --flush

echo "Starting grading"


for FILE_NAME in ${GRAPH_DISP}; do
    echo ${FILE_NAME}
    diff  ${TOPO_DIR}${TOPO_FILE_PREFIX}${FILE_END} ${FILE_NAME}
done
