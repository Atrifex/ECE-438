#!/bin/bash

# Settings
DEFAULT_TOPO_DIR="./logfiles/"
DEFAULT_TOPO_FILE_PREFIX="graph"

# Command line parameters
TOPO_DIR=${DEFAULT_TOPO_DIR}
TOPO_FILE_PREFIX=${DEFAULT_TOPO_FILE_PREFIX}
GRAPH_DISP=$(find ${TOPO_DIR} -maxdepth 1 -name "${TOPO_FILE_PREFIX}*" -print)

GOLD_FILE=${1:-"./example_topology/goldNetwork.txt"}

# pkill ls_router
# sudo iptables --flush

echo "Starting grading:"

TOPO_PASSED=1
FILES_EXIST=0
for FILE_NAME in ${GRAPH_DISP}; do
    FILES_EXIST=1
    NODE_ID=${FILE_NAME#*${TOPO_FILE_PREFIX}}
    if ! diff -q "${GOLD_FILE}" "${FILE_NAME}"; then
        echo "${TOPO_DIR}graph${NODE_ID} differs from gold output"
        TOPO_PASSED=0
    fi
done

if [ ${TOPO_PASSED} -eq 1 ] && [ ${FILES_EXIST} -eq 1 ]; then
    echo "All output files converged. PASS!"
else
    echo "Files did not converge. Make sure executable is generating graph* files."
    echo "FAIL!"
fi
