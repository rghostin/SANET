#!/bin/bash

ROOT='/home/ubuntu/sanet'
BATMAN_PATH="$ROOT/batman"
GLOBALS_SCRIPT="$BATMAN_PATH/globals.sh"

set -e

source "$GLOBALS_SCRIPT"



cd "$ROOT"

# force run as root
if [[ $EUID -ne 0 ]]; then
   echo "This script must be run as root"
   exit 1
fi
    echo "- setting static IP address=$P_IP_ADDR/24"
    ip addr add dev "$P_IFACE" "$P_IP_ADDR"/24

    echo '[*] Overwriting dhcpd configuration'
    mv "$RSRC_DIR/dhcpd.conf" "/etc/dhcp/dhcpd.conf"

    echo '[*] Overwriting isc-dhcp-server default configuration'
    mv "$RSRC_DIR/isc-dhcp-server" "/etc/default/isc-dhcp-server"

    echo '[*] Restarting DHCP server'
    systemctl unmask isc-dhcp-server
    service isc-dhcp-server restart

    echo '[*] Overwriting hostapd configuration'
    mv "$RSRC_DIR/hostapd.conf" "/etc/hostapd/hostapd.conf"

    echo '[*] Overwriting hostapd default configuration'
    mv "$RSRC_DIR/hostapd" "/etc/default/hostapd"

    systemctl unmask hostapd
    service hostapd restart

    echo '[*] Overwriting sysctl configuration'
    mv "$RSRC_DIR/sysctl.conf" "/etc/sysctl.conf"

    echo '[*] Activation of forwarding'
    sh -c "echo 1 > /proc/sys/net/ipv4/ip_forward"

echo '[*] Done'

