#!/bin/bash
ROOT='/home/ubuntu/sanet'
BATMAN_PATH="$ROOT/batman"
GLOBALS_SCRIPT="$BATMAN_PATH/globals.sh"

source "$GLOBALS_SCRIPT"


NODEID_CFG_PATH="$ROOT/conf/nodeid.conf"


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
