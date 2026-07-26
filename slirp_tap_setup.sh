#!/bin/bash

#see https://wiki.qemu.org/Documentation/Networking : Network HOWTOs
#https://unix.stackexchange.com/questions/261801/putting-a-network-interface-up-down-from-command-line


if [ "$EUID" -ne 0 ]; then
	echo "run with sudo"
	exit 1
fi

OWNER="${SUDO_USER:-$(logname)}"

ip link set tap0 down 2>/dev/null
ip tuntap del dev tap0 mode tap 2>/dev/null
ip link set br0 down 2>/dev/null
ip link delete br0 type bridge 2>/dev/null

#creates a tun bridge between the OS and host's physical networking device
ip link add br0 type bridge
ip tuntap add dev tap0 mode tap user "$OWNER"
ip link set tap0 master br0
ip addr add 10.0.2.1/24 dev br0
ip link set tap0 up
ip link set br0 up

#allows virtual nic to communicate with real router and perform hops
sudo nft add table ip nat && sudo nft 'add chain ip nat postrouting { type nat hook postrouting priority 100 ; }' && sudo nft add rule ip nat postrouting ip saddr 10.0.2.0/24 oif eth0 masquerade
sysctl -w net.ipv4.ip_forward=1
