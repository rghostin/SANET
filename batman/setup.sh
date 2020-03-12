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


function usage() {
	echo "This script must be run with super-user privileges.
	Usage: $0 [arguments]
	-i NodeID (0 <= i <= 254)
  -h Hostname
  bool_Antenna (0 : to enable | 1 : to disable)"
}

cd "$ROOT"

# force run as root
if [[ $EUID -ne 0 ]]; then
   echo "This script must be run as root"
   exit 1
fi

# check params
if [[ $# -lt 2 ]]; then
  usage
  exit 1
fi

#Antenna_off
a=1

# getting params
while getopts ":h:i:a:" o; do
    case "${o}" in
        i)
            i=${OPTARG}
            (( i >= 0 && i <= 254 )) || usage
            ;;
        h)
            h=${OPTARG}
            ;;
        a)
            a=${OPTARG}
            ;;
        *)
            usage
            ;;
    esac
done
shift $((OPTIND-1))

if [ -z "${i}" ] || [ -z "${a}" ]; then
    usage
    exit 1
fi

echo "i = ${i}"
echo "h = ${h}"
echo "a = ${a}"

echo "[*] Setting timestamp"
timedatectl set-timezone Europe/Brussels

echo "changing hostname to $h"
hostnamectl set-hostname "$h"

echo '[*] Writing NodeID configuration'
echo -n "$i" > "$NODEID_CONF"

echo '[*] Connecting to wifi'
"$CONNECTWIFI_SCRIPT"

echo '[*] Updating server'
apt update
apt upgrade -y
apt autoremove -y

echo '[*] Installing requirements'
# opencv : build-essential cmake libgtk2.0-dev pkg-config libavcodec-dev libavformat-dev libswscale-dev libopencv-dev
apt install wireless-tools sqlite3 libsqlite3-dev iw batctl alfred make g++ python3-setuptools python3-pip libssl-dev libgeos++-dev hostapd isc-dhcp-server -y

echo '[*] Installing Python3 - requirements'
pip3 install -r "$RSRC_DIR/requirements_rasp.txt" || exit 1

echo "[*] Setting keyboard layout to $KBLAYOUT"
loadkeys "$KBLAYOUT"
changeKeyboard "$KBLAYOUT"

echo '[*] Copying screen config'
# copy rsrc/config* to /boot/firmware/
cp "$RSRC_DIR"/config_std.txt /boot/firmware/
cp "$RSRC_DIR"/config_waveshare.txt /boot/firmware/

echo "[*] Compiling robin"
make rebuild

#echo '[*] Enabling batman and robin on reboot'
#echo "@reboot   root    ${RUN_SCRIPT}" >> /etc/crontab

if [[ "$a" -eq "0" ]] ; then  # Enabling Antenna
    "$ACTIVATE_ANTENNA_SCRIPT"
else
    systemctl stop isc-dhcp-server
    systemctl mask isc-dhcp-server
    systemctl stop hostapd
    systemctl mask hostapd
fi

echo '[*] Done'

echo '[*] Reboot to apply changes'
sleep 1
reboot
