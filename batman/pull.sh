#!/bin/bash
ROOT='/home/ubuntu/sanet'
BATMAN_PATH="$ROOT/batman"
GLOBALS_SCRIPT="$BATMAN_PATH/globals.sh"

set -e

source "$GLOBALS_SCRIPT"

cd "$ROOT"
"$CONNETCTWIFI_SCRIPT"
git pull --all
