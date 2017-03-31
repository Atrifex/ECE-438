#!/bin/bash

# Settings
DEFAULT_TOPO_DIR="./logfiles/"
DEFAULT_TOPO_FILE_PREFIX="graph"

# Command line parameters
TOPO_DIR=${DEFAULT_TOPO_DIR}
TOPO_FILE_PREFIX=${DEFAULT_TOPO_FILE_PREFIX}
GRAPH_DISP=$(find ${TOPO_DIR} -maxdepth 1 -name "${TOPO_FILE_PREFIX}*" -print)

GOLD_FILE=${1:-"./example_topology/goldNetwork.txt"}

if [ ${FILE_END} -eq -1 ]; then
    echo "Specify node to compare to" 
    exit 1
fi

pkill ls_router
sudo iptables --flush

echo "Starting grading:"

TOPO_PASSED=1
for FILE_NAME in ${GRAPH_DISP}; do
    if ! diff -q "${GOLD_FILE}" "${FILE_NAME}"; then
        echo "${FILE_NAME} differs from gold output"
        TOPO_PASSED=0
    fi
done

if [ ${TOPO_PASSED} -eq 1 ]; then
    echo "All output files converged. PASS!"
else
    echo "Files did not converge. FAIL!"
fi
