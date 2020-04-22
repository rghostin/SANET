#!/bin/bash
ROOT='/home/ubuntu/sanet'
BATMAN_PATH="$ROOT/batman"
GLOBALS_SCRIPT="$BATMAN_PATH/globals.sh"

source "$GLOBALS_SCRIPT"

if [[ $# -le 0 ]]; then
   echo "Missing nodeid"
   exit 1
fi

timedatectl set-timezone Europe/Brussels

echo "Disconnect ethernet"
sleep 15

if [[ $# -eq 2 ]]; then
   echo "Antenna mode on"
   "$BATMAN_PATH/antenna.sh"
fi

echo "Batmanify"
"$BATMANIFY_SCRIPT" "$1"

echo "Launch now"
"$ROOT/launch.sh" "3"
