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

function compileLPIGPIO {
    cd "$DIRLPGPIO"
    "make"
    "make" "install"
    echo "Pigpio INSTALLED"

    echo "TESTING Pigpio"
    "./x_pigpio"

    echo "COMPILING Pigpio"
    gcc -Wall -pthread  "${TESTGPIO}.c" -o "$TESTGPIO" -lpigpio -lrt

    echo "DONE : Dependencies installed"
    cd "$ROOT"
}

function compileRobin {
  cd "$ROBIN_SRC_DIR"
  make rebuild
  cd "$ROOT"
}

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

echo "changing hostname to $2"
hostnamectl set-hostname "$2"

echo '[*] Connecting to wifi'
"$CONNETCTWIFI"

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
echo -n "$1" > "$HOSTID_CONF"

echo "[*] Compiling LPIGPIO"
compileLPIGPIO

echo "[*] Compiling robin"
compileRobin

echo '[*] Setting up batman'
chmod +x "$BATMANIFY_SCRIPT"
"$BATMANIFY_SCRIPT"  # --hostid-conf "$HOSTID_CONF"

echo '[*] Enabling batman and robin on reboot'
echo "@reboot   root    $RUN" >> /etc/crontab
#g(crontab -l 2>/dev/null; echo "@reboot $RUN") | crontab -

echo '[*] Starting Robin'
"$ROBIN"

echo '[*] Done'
