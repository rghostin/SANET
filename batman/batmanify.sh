#!/bin/bash

ROOT='/home/ubuntu/sanet'
BATMAN_PATH="$ROOT/batman"
GLOBALS_SCRIPT="$BATMAN_PATH/globals.sh"

SLEEP_PERIOD='2'

set -e

source globals.sh


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


# params validation
# param1 : nodeid
if [[ ! $1 =~ ^[0-9]+$ ]]; then
  echo "Error: param nodeid must be an integer"
  exit 1
fi
NODEID="$1"

# clean up
echo "[*] Cleaning up"
systemctl stop wpa_supplicant
systemctl mask wpa_supplicant
"$KILLWIFI_SCRIPT"

#  Mesh wireless configuration
echo "[*] Setting up mesh wireless interface"
echo "- Shutting down $W_IFACE"
ip link set dev "$W_IFACE" down
sleep "$SLEEP_PERIOD"
echo "- Setting IBSS mode"
iw dev wlan0 set type ibss
echo "- Setting MTU $MESH_W_CHANNEL"
ip link set dev "$W_IFACE" mtu "$MESH_W_MTU"
echo "- Setting channel $MESH_W_CHANNEL"
iwconfig "$W_IFACE" channel "$MESH_W_CHANNEL"
echo "- Setting $W_IFACE up"
ip link set dev "$W_IFACE" up
echo "- Joining $MESH_W_ESSID wireless" 
iw "$W_IFACE" ibss join "$MESH_W_ESSID" "$MESH_W_FREQ"

# batman configuration
echo "[*] Setting up batman"
echo "- Activating batman-adv kernel module"
modprobe batman-adv
echo "- Adding $W_IFACE to batman-adv"
batctl if add "$W_IFACE" 
ip link set dev "$W_IFACE" up
echo " - Setting $B_IFACE up"
ip link set dev "$B_IFACE" up
echo "- setting static IP address=10.8.0.$1/24"
ip addr add dev "$B_IFACE" 10.8.0."$NODEID"/24

#
echo "[*] Done"

