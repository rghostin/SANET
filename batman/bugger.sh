#!/bin/bash
ROOT='/home/ubuntu/sanet'
BATMAN_PATH="$ROOT/batman"
GLOBALS_SCRIPT="$BATMAN_PATH/globals.sh"

source "$GLOBALS_SCRIPT"


start_global=$((date +%s))
for i in $(seq 1 24); do    
    start_iter=$((date +%s))
    
    "$CONNECTWIFI_SCRIPT"
    sleep 5
    "$BATMANIFY_SCRIPT"

    end_iter=$((date +%s))
    runtime_iter=$((end_iter-start_iter))
    echo "iteration runtime $i: $runtime_iter"
done
end_global=$((date +%s))

runtime_global=$((end_global-start_global))
echo "global runtime: $runtime_global"
