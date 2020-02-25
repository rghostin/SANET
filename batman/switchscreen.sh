#!/bin/bash

set -e 

#PATHS
FIR="/boot/firmware"

#POSSIBLE CONFIGURATIONS
STD='std'
WVS='waveshare'

if [[ $# != 1 ]]; then
  echo "[!] Error usage: setup.sh '$STD' or '$WVS'"
  exit 1
fi

if [ "$1" == "$STD" ]; then
    echo "[*] Setting up standard screen configuration"
    cp "$FIR"/config_std.txt "$FIR"/config.txt
elif [ "$1" == "$WVS" ]; then
    echo "[*] Setting up waveshare screen configuration"
    cp "$FIR"/config_waveshare.txt "$FIR"/config.txt
fi