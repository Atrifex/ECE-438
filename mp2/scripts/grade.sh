#!/bin/bash

# Settings
DEFAULT_TOPO_DIR="./logfiles/"
DEFAULT_TOPO_FILE_PREFIX="log"

# Command line parameters
TOPO_DIR=${DEFAULT_TOPO_DIR}
TOPO_FILE_PREFIX=${DEFAULT_TOPO_FILE_PREFIX}
GRAPH_DISP=$(find ${TOPO_DIR} -maxdepth 1 -name "${TOPO_FILE_PREFIX}*" -print)

GOLD_FILE=${1:-"./example_topology/goldNetwork.txt"}

pkill ls_router
sudo iptables --flush

echo "Starting grading:"

TOPO_PASSED=1
for FILE_NAME in ${GRAPH_DISP}; do
    NODE_ID=${FILE_NAME#*${TOPO_FILE_PREFIX}}
    if ! diff -q "${GOLD_FILE}" "${TOPO_DIR}graph${NODE_ID}"; then
        echo "${FILE_NAME} differs from gold output"
        TOPO_PASSED=0
    fi
done

if [ ${TOPO_PASSED} -eq 1 ]; then
    echo "All output files converged. PASS!"
else
    echo "Files did not converge. FAIL!"
fi
