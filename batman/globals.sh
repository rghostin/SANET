ROOT='/home/ubuntu/sanet'

# network settings
W_IFACE="wlan0"
B_IFACE="bat0"
MESH_W_ESSID="meshnetw"
MESH_W_CHANNEL="3"	# TODO
MESH_W_FREQ="2432"
MESH_W_MTU="1500"
CONN_IDENTITY="hlloreda@ulb.ac.be"
CONN_PASSWORD="^nNxNx6k"

# general settings
KBLAYOUT="be"

# paths
ROBIN_PROG="$ROOT/robin"

BATMAN_PATH="$ROOT/batman"
TESTGPIO="$BATMAN_PATH/testgpio"
BATMANIFY_SCRIPT="$BATMAN_PATH/batmanify.sh"
RSRC_DIR="$BATMAN_PATH/rsrc"
CONNECTWIFI_SCRIPT="$BATMAN_PATH/connectwifi.sh"
KILLWIFI_SCRIPT="$BATMAN_PATH/killwifi.sh"
DIRLPGPIO="$ROBIN_SRC_DIR/lib/pigpio-master"
RUN="$ROOT/run.sh"
