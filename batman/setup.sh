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
	  - i:NodeID (0 <= i <= 254)
    - h:Hostname
    - a:Antenna (0 : to enable | 1 : to disable)"
}

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

echo '[*] Installing  requirements'
apt install wireless-tools iw batctl alfred make g++ python3-setuptools libssl-dev hostapd isc-dhcp-server -y

echo "[*] Setting keyboard layout to $KBLAYOUT"
loadkeys "$KBLAYOUT"
changeKeyboard "$KBLAYOUT"

echo '[*] Copying screen config'
# copy rsrc/config* to /boot/firmware/
cp "$RSRC_DIR"/config_std.txt /boot/firmware/
cp "$RSRC_DIR"/config_waveshare.txt /boot/firmware/

echo "[*] Compiling robin"
make rebuild

echo '[*] Enabling batman and robin on reboot'
echo "@reboot   root    ${RUN_SCRIPT}" >> /etc/crontab

if [[ $a -eq 0 ]] ; then  # Enabling Antenna
    echo "- setting static IP address=$P_IP_ADDR/24"
    ip addr add dev "$P_IFACE" "$P_IP_ADDR"/24

    echo '[*] Writing dhcpd configuration'
    #dhcpd_file="/etc/dhcp/dhcpd.conf"  # Todo cp file

    # Todo cp file -> /etc/default/isc-dhcp-server

    echo '[*] Restarting DHCP server'
    service isc-dhcp-server restart

    # Todo cp file -> /etc/hostapd/hostapd.conf + /etc/default/hostapd

    echo '[*] Activation of forwarding'
    sh -c "echo 1 > /proc/sys/net/ipv4/ip_forward"
else
    systemctl mask isc-dhcp-server
    systemctl mask hostapd
fi

echo '[*] Done'