#!/bin/bash

BRIDGE_NAME="chpokbr"
TAP_NAME_PREFIX="chpoktap"

if [ $# -gt 2 ] || [ $# -lt 1 ]; then
    echo "Usage: $0 NUM [USER]" >&2
    echo "Create NUM tap interfaces, owned by USER, joined by a bridge." >&2
    echo "Supply NUM=0 to clean up everything." >&2
    echo "If USER is not supplied, it defaults to \$SUDO_UID or \$UID (whichever available)" >&2
    exit 1
fi

NUM=$1
OWNER=$2 

if [ $EUID -ne 0 ]; then
    echo "Script must be run as root (use sudo or something)" >&2
    exit 1
fi

if [ -z "$OWNER" ]; then
    OWNER=$SUDO_UID
fi
if [ -z "$OWNER" ]; then
    OWNER=$UID
fi

if ! getent passwd $OWNER >/dev/null; then
    echo "User $OWNER doesn't exist" >&2
    exit 1
fi

if ! test "$NUM" -eq "$NUM" 2>/dev/null; then
    echo "Argument '$NUM' is not a number" >&2
    exit 1
fi

OWNER_NAME=$(getent passwd $OWNER | awk -F: '{ print $1 }')
echo "Configuring tap interfaces for user $OWNER_NAME..." >&2

destroy_bridge() {
    for br in $(ls /sys/class/net | grep $BRIDGE_NAME); do
        echo "Destroying $br..." >&2
        ip link delete $br
    done
}

destroy_taps() {
    for tap in $(ls /sys/class/net | grep ^$TAP_NAME_PREFIX); do
        echo "Destroying $tap..." >&2
        ip link delete $tap
    done
}


create_bridge() {
    echo "Creating $BRIDGE_NAME..." >&2
    ip link add $BRIDGE_NAME type bridge
    ip link set $BRIDGE_NAME up
}

create_tap() {
    TAP_NAME=$TAP_NAME_PREFIX$1

    echo "Creating $TAP_NAME..." >&2
    ip tuntap add $TAP_NAME mode tap user $OWNER || exit 1
    ip link set $TAP_NAME master $BRIDGE_NAME
    ip link set $TAP_NAME up
}

destroy_taps
destroy_bridge

if [ $NUM -gt 2 ]; then
    create_bridge
fi


for i in `seq 0 $(expr $NUM - 1)`; do
    create_tap $i
    

done

