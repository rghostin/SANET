#!/bin/bash

NODEID_CFG_PATH="conf/nodeid.conf"


if [[ $# -ne 1 ]]; then
    echo "Error: usage setnodeid nodeid"
    exit 1
fi

if ! (( "$1" >= 0 && "$1" <= 254 )); then
    echo "Error: nodeid must been between 0 and 255"
    exit 1
fi

echo "$1" > "${NODEID_CFG_PATH}"
echo "${NODEID_CFG_PATH} set nodeid=$1"
