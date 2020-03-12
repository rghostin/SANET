#!/bin/bash
ROOT='/home/ubuntu/sanet'
BATMAN_PATH="$ROOT/batman"
GLOBALS_SCRIPT="$BATMAN_PATH/globals.sh"

set -e

source "$GLOBALS_SCRIPT"

function usage() { echo "Usage: $0 [-i <0-254>]" 1>&2; exit 1; }


# force run as root
if [[ $EUID -ne 0 ]]; then
   echo "This script must be run as root" 
   exit 1
fi

#verbosity
v=3

while getopts ":i:v:" o; do
    case "${o}" in
        i)
            i=${OPTARG}
            (( i >= 0 && i <= 254 )) || usage
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

echo "i = ${i}"
echo "v = ${v}"

echo "{*} Starting batmanify script"
"$BATMANIFY_SCRIPT" "$i"
echo "{*} Setting nodeid : $i"
"$SET_NODEID" "$i"
echo "{*} Starting robin and flight_srv program"
$TMX_START "$v"
