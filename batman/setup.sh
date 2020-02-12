#!/bin/bash

ROOT='/home/ubuntu/sanet'
BATMAN_PATH="$ROOT/batman"
GLOBALS_SCRIPT="$BATMAN_PATH/globals.sh"

set -e

source "$GLOBALS_SCRIPT"


function changeKeyboard {
    newValue="$1" 
    sed -i "s/\(XKBLAYOUT *= *\).*/\1\"$newValue\"/" "/etc/default/keyboard"
    echo " - Keyboard has been changed to $newValue"
}


function usage() {}

cd "$ROOT"

# force run as root
if [[ $EUID -ne 0 ]]; then
   echo "This script must be run as root" 
   exit 1
fi

# check params
if [[ $# != 2 ]]; then
  echo "[!] Error usage: setup.sh hostid hostname"
  exit 1
fi

# getting params
while getopts ":h:i:" o; do
    case "${o}" in
        i)
            i=${OPTARG}
            (( i >= 0 && i <= 254 )) || usage
            ;;
        h)
            h=${OPTARG}
            ;;
        *)
            usage
            ;;
    esac
done
shift $((OPTIND-1))


echo "changing hostname to $h"
hostnamectl set-hostname "$h"

echo '[*] Connecting to wifi'
"$CONNECTWIFI_SCRIPT"

echo '[*] Updating server'
apt update
apt upgrade -y
apt autoremove -y

echo '[*] Installing  requirements'
apt install wireless-tools iw batctl alfred make g++ python3-setuptools -y

echo '[*] Setting keyboard layout to FR'
changeKeyboard "$KBLAYOUT"

echo '[*] Copying screen config'
# copy rsrc/config* to /boot/firmware/
cp "$RSRC_DIR"/config_std.txt /boot/firmware/
cp "$RSRC_DIR"/config_waveshare.txt /boot/firmware/

echo '[*] Writing hostid configuration'
echo -n "$i" > "$NODEID_CONF"

echo "[*] Compiling robin"
make rebuild

echo '[*] Enabling batman and robin on reboot'
echo "@reboot   root    ${RUN_SCRIPT}" >> /etc/crontab

echo '[*] Done'
