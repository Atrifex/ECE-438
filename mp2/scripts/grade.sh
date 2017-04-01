#!/bin/bash

# Settings
DEFAULT_TOPO_DIR="./log/"
DEFAULT_TOPO_FILE_PREFIX="graph"

# Command line parameters
TOPO_DIR=${DEFAULT_TOPO_DIR}
TOPO_FILE_PREFIX=${DEFAULT_TOPO_FILE_PREFIX}
GRAPH_DISP=$(find ${TOPO_DIR} -maxdepth 1 -name "${TOPO_FILE_PREFIX}*" -print)

BASE_NODE=${1:-"0"}

killall ls_router
sudo iptables --flush

echo "Starting grading:"

TOPO_PASSED=1
FILES_EXIST=0
for FILE_NAME in ${GRAPH_DISP}; do
    FILES_EXIST=1
    NODE_ID=${FILE_NAME#*${TOPO_FILE_PREFIX}}
    if ! diff -q "${TOPO_DIR}${TOPO_FILE_PREFIX}${BASE_NODE}" "${FILE_NAME}"; then
        echo "${TOPO_DIR}graph${NODE_ID} differs from base node file"
        TOPO_PASSED=0
        break
    fi
done

if [ ${TOPO_PASSED} -eq 1 ] && [ ${FILES_EXIST} -eq 1 ]; then
    echo "All output files converged."
    echo "PASS!"
else
    if [ ${FILES_EXIST} -eq 0 ]; then
        echo "Make sure executable is generating graph* files."
        echo "NO RESULTS."
    else
        echo "Files did not converge."
        echo "FAIL!"
    fi
fi
