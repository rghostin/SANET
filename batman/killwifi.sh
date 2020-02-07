#!/bin/bash
W_IFACE="wlan0"
B_IFACE="bat0"

function check_interface_exists {
    local interface="$1"
    local res_cat

    # if file exists then the interface exist
    if [[ -e "/sys/class/net/$interface/operstate" ]] ; then
          return 0
    else
       return 1
    fi
}


# force run as root
if [[ $EUID -ne 0 ]]; then
   echo "This script must be run as root" 
   exit 1
fi

dhclient -r "$W_IFACE"
killall -9 dhclient || echo "No process dhclient to kill"
ip link set "$W_IFACE" down
killall -9 wpa_supplicant || echo "No process wpa_supplicant to kill"

# kill batman if exists
check_interface_exists "$B_IFACE"
if [[ $? -eq 0 ]]; then
  ip link set dev "$B_IFACE" down
fi
