#!/bin/bash
ROOT='/home/ubuntu/sanet'
BATMAN_PATH="$ROOT/batman"
GLOBALS_SCRIPT="$BATMAN_PATH/globals.sh"

source "$GLOBALS_SCRIPT"

if [[ $# -ne 1 ]]; then
   echo "Missing nodeid"
   exit 1
fi

timedatectl set-timezone Europe/Brussels

if [[ $# -ne 2 ]]; then
   echo "Antenna mode on"
   "$BATMAN_PATH/antenna.sh"
fi

echo "Batmanify"
"$BATMANIFY_SCRIPT" "$1"

echo "Launch now"
"$BATMAN_PATH/launch.sh" "3"