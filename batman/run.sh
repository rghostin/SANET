#!/bin/bash
ROOT='/home/ubuntu/sanet'
BATMAN_PATH="$ROOT/batman"
GLOBALS_SCRIPT="$BATMAN_PATH/globals.sh"

set -e

source "$GLOBALS_SCRIPT"

# force run as root
if [[ $EUID -ne 0 ]]; then
   echo "This script must be run as root" 
   exit 1
fi

# check number of params
if [[ $# -ne 1 ]]; then
   echo "Error: Invalid number of arguments"
   exit 1
fi

"$BATMANIFY_SCRIPT" "$1"
"$ROBIN_PROG" -i "$1"