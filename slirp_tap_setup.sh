#!/bin/bash

#see https://wiki.qemu.org/Documentation/Networking : Network HOWTOs
#https://unix.stackexchange.com/questions/261801/putting-a-network-interface-up-down-from-command-line

#creates a tun bridge between the OS and host's physical networking device

ip link add br0 type bridge
ip tuntap add dev tap0 mode tap
ip link set dev tap0 master br0   # set br0 as the target bridge for tap0
ip link set dev eth0 master br0   # set br0 as the target bridge for eth0
ip link set dev br0 up

#bridge works but is not usable as it does not have an IP address

ip link set tap0 master br0 
ip addr replace 10.0.2.1/24 dev br0
