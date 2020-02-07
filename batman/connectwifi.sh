#!/bin/bash

B_IFACE="bat0"

# Connection settings
CONN_IDENTITY="hlloreda@ulb.ac.be"
CONN_PASSWORD="^nNxNx6k"

KILLWIFI_SCRIPT="killwifi.sh"

set -e

function check_connectivity {
    local test_ip
    local test_count

    test_ip="8.8.8.8"
    test_count=1

    ping -c ${test_count} ${test_ip} > /dev/null
    return $?
}

# force run as root
if [[ $EUID -ne 0 ]]; then
   echo "This script must be run as root" 
   exit 1
fi


# disconnect wifi
./"$KILLWIFI_SCRIPT"

# Connect to Wifi
echo '[*] Connecting to wifi'

# writing wifi connection config
cat <<EOT >> /etc/wpa_suplicant.conf
network={
	ssid="eduroam"
	priority=1
	proto=RSN
	key_mgmt=WPA-EAP
	pairwise=CCMP
	auth_alg=OPEN
	eap=PEAP
	identity="$CONN_IDENTITY"
	password="$CONN_PASSWORD"
	phase1="peaplabel=0"
	phase2="auth=MSCHAPV2"
}
EOT

wpa_supplicant -B -c /etc/wpa_supplicant.conf -i wlan0
dhclient wlan0
