#!/bin/bash

ROOT='/home/ubuntu/sanet'
BATMAN_PATH="$ROOT/batman"
GLOBALS_SCRIPT="$BATMAN_PATH/globals.sh"

source "$GLOBALS_SCRIPT"

function changeValueLine {
    local target_key="$1" 
    local replacement_value="$2"
    local config_file="$3"
    local bool_quote="$4"

    if [ "$bool_quote" -eq 0 ] ; then
        sed -c -i "s/\($target_key *= *\).*/\1\"$replacement_value\"/" $config_file
    else
        sed -c -i "s/\($target_key *= *\).*/\1$replacement_value/" $config_file
    fi

    echo " - Following setting $target_key has been changed to $replacement_value"
}

function uncommentLine {
    local target_key="$1" 
    local replacement_value="${TARGET_KEY#\#}"
    local config_file="$2"

    sed -i "s/$target_key/$replacement_value/g" "$config_file"
    echo " - $target_key was uncommented in $config_file"
}

function commentLine {
    local target_key="$1" 
    local replacement_value="#$TARGET_KEY"
    local config_file="$2"

    sed -i "s/$target_key/$replacement_value/g" "$config_file"
    echo " - $target_key was commented in $config_file"
}

# force run as root
if [[ $EUID -ne 0 ]]; then
   echo "This script must be run as root" 
   exit 1
fi

# check params
if [[ $# != 2 ]]; then
  echo "[!] Error usage: getpublic.sh nodeid interface"
  exit 1
fi

# getting params
while getopts ":h:i:" o; do
    case "${o}" in
        i)
            i=${OPTARG}
            (( i >= 0 && i <= 254 )) || usage  # TODO check usage
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

NODEID="$i"
P_IFACE="$h" # TODO check + getopt ?

#echo '[*] Writing /etc/network/interfaces configuration'
#cat <<EOT >> /etc/network/interfaces
#allow-hotplug $P_IFACE

#iface $P_IFACE inet static
#address $P_IP_ADDR
#netmask 255.255.255.0
#EOT

echo "- setting static IP address=$P_IP_ADDR/24"
ip addr add dev "$P_IFACE" "$P_IP_ADDR"/24

echo '[*] Installing hostapd and isc-dhcp-server'
apt install hostapd isc-dhcp-server


echo '[*] Writing dhcpd configuration'
dhcpd_file="/etc/dhcp/dhcpd.conf"
commentLine "option domain-name " "$dhcpd_file"
commentLine "option domain-name-servers" "$dhcpd_file"
uncommentLine "#authoritative" "$dhcpd_file"


cat <<EOT >> $dhcpd_file
subnet 192.168.42.0 netmask 255.255.255.0 {
range 192.168.42.10 192.168.42.50;
option broadcast-address 192.168.42.255;
option routers 192.168.42.1;
default-lease-time 600;
max-lease-time 7200;
option domain-name "local";
option domain-name-servers 8.8.8.8, 8.8.4.4;
}
EOT

changeValueLine "INTERFACES" "$P_IFACE" "/etc/default/isc-dhcp-server" "0"
changeValueLine "INTERFACES" "$P_IFACE" "/etc/default/isc-dhcp-server" "0"


echo '[*] Restarting DHCP server'
service isc-dhcp-server restart


echo '[*] Writing hostapd configuration'
cat <<EOT >> /etc/hostapd/hostapd.conf
interface=$P_IFACE
driver=nl80211
ssid=$P_WIRELS_SSID
hw_mode=g
channel=6
macaddr_acl=0
auth_algs=1
ignore_broadcast_ssid=0
wpa=2
wpa_passphrase=$P_WIRELS_PSWD
wpa_key_mgmt=WPA-PSK
wpa_pairwise=TKIP
rsn_pairwise=CCMP
EOT


echo '[*] Uncommenting line in hostapd.conf'
uncommentLine "#DAEMON_CONF" "/etc/default/hostapd"
echo '[*] Changing value in hostapd.conf'
changeValueLine "DAEMON_CONF" "/etc/hostapd/hostapd.conf" "/etc/default/hostapd"

echo '[*] Uncommenting line in sysctl.conf'
uncommentLine "#net.ipv4.ip_forward=1" "/etc/sysctl.conf"

echo '[*] Activation of forwarding'
sudo sh -c "echo 1 > /proc/sys/net/ipv4/ip_forward"