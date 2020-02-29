ROOT='/home/ubuntu/sanet'

# network settings
W_IFACE="wlan0"
B_IFACE="bat0"
P_IFACE="wlan1"
MESH_W_ESSID="meshnetw"
MESH_W_CHANNEL="3"	# TODO research
MESH_W_FREQ="2432"
MESH_W_MTU="1500"
CONN_IDENTITY="gahz7w@guestroam.be"
CONN_PASSWORD="Ahngaen9"

# Public network settings
P_WIRELS_SSID="AMD_PI"
P_WIRELS_PSWD="raspberryNOPC"
P_IP_ADDR="192.168.42.1"

# general settings
KBLAYOUT="be"

# paths
ROBIN_PROG="$ROOT/robin"
FP_DIR="$ROOT/flightplanning"

BATMAN_PATH="$ROOT/batman"
BATMANIFY_SCRIPT="$BATMAN_PATH/batmanify.sh"
RSRC_DIR="$BATMAN_PATH/rsrc"
CONF_DIR="$ROOT/conf"
CONNECTWIFI_SCRIPT="$BATMAN_PATH/connectwifi.sh"
KILLWIFI_SCRIPT="$BATMAN_PATH/killwifi.sh"
RUN_SCRIPT="$BATMAN_PATH/run.sh"
NODEID_CONF="$CONF_DIR/nodeid.conf"
TMX_START="$ROOT/launch.sh"