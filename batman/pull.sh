#!/bin/bash
ROOT='/home/ubuntu/sanet'
BATMAN_PATH="$ROOT/batman"
GLOBALS_SCRIPT="$BATMAN_PATH/globals.sh"

set -e

source "$GLOBALS_SCRIPT"

function usage() { echo "Usage: $0 [-r]" 1>&2; exit 1; }


flag_rebuild='false'
while getopts ":r" o; do
    case "${o}" in
        r)
            flag_rebuild='true'
            ;;
        *)
            usage
            ;;
    esac
done
shift $((OPTIND-1))

cd "$ROOT"
"$CONNECTWIFI_SCRIPT"
git pull --all


if ${flag_rebuild}; then
    echo "Rebuilding source"
    make rebuild
else
    echo "No rebuild flag specified"
fi
