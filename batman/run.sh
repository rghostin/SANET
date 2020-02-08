#!/bin/bash
ROOT='/home/ubuntu/sanet'
BATMAN_PATH="$ROOT/batman"
GLOBALS_SCRIPT="$BATMAN_PATH/globals.sh"

set -e

source "$GLOBALS_SCRIPT"

function usage() { echo "Usage: $0 [-i <0-254>] [-v <-2-9>]" 1>&2; exit 1; }


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

while getopts ":i:v:" o; do
    case "${o}" in
        i)
            i=${OPTARG}
            (( s >= 0 && s <= 254 )) || usage
            ;;
        v)
            v=${OPTARG}
            (( v >= -2 && v <= 9 )) || usage
            ;;
        *)
            usage
            ;;
    esac
done
shift $((OPTIND-1))

if [ -z "${i}" ] || [ -z "${v}" ]; then
    usage
fi

echo "i = ${s}"
echo "v = ${p}"

"$BATMANIFY_SCRIPT" "$1"
"$ROBIN_PROG" -i "$1"