#!/bin/sh

# This script is for use with the uIP test programs and AtariSIO.
# Before running this, run "make disk" to create the disk image.
# This script needs to be run with root privileges (e.g. with sudo)

# Configurable stuff:

ATR_IMAGE=${1-fujichat.atr}

#ATR_IMAGE=fujitest.atr # Name of the disk image
TTY=/dev/ttyS0         # serial port to use
BAUD=4800              # must match compiled-in value in rs232dev.c
SLATTACH=a8_slattach   # slattach binary (possibly patched)
SLIP_IFACE=sl0         # probably no need to change this
LOCAL_IP=192.168.0.1   # SLIP IP address for Linux host
REMOTE_IP=192.168.0.2  # SLIP IP address for Atari host
IP_FORWARD=yes         # Route packets for the Atari?
IP_MASQUERADE=yes      # NAT for the Atari?
MASQ_IFACE=ra0         # if NATing, our main (LAM or internet) interface
DUMP_PACKETS=yes       # Run tcpdump on sl0 interface?

# DELAY is needed on some (most?) systems because e.g. atariserver tries
# to run before the atarisio module is fully initialized. Choose one:

# No delay:
#DELAY="true"

# 1 second:
#DELAY="sleep 1"

# 1/4 second (may not work on old Linux installs):
DELAY="usleep 250000"

# End of config section, start of code:

ifconfig $SLIP_IFACE down 2&>/dev/null
killall $SLATTACH 2&>/dev/null
fuser -k $TTY 2&>/dev/null # make sure nobody's using the port...
$DELAY
fuser -k -9 $TTY 2&>/dev/null # Just in case...
$DELAY
modprobe atarisio port=$TTY
$DELAY
atariserver $ATR_IMAGE
#atariserver autorun.sys # fails when run from MyPicoDOS, why?
$DELAY
rmmod atarisio
$DELAY
echo "Starting SLIP on $SLIP_IFACE, local $LOCAL_IP, remote $REMOTE_IP"
$SLATTACH -L -p slip -s $BAUD $TTY &
$DELAY
ifconfig $SLIP_IFACE $LOCAL_IP mtu 576
ifconfig $SLIP_IFACE $LOCAL_IP pointopoint $REMOTE_IP

if [ "$IP_MASQUERADE" = "yes" ]; then
	echo "IP Masquerading enabled"
	iptables -F
	iptables -t nat -F
	iptables -t nat -A POSTROUTING -o $MASQ_IFACE -j MASQUERADE
fi

if [ "$IP_FORWARD" = "yes" ]; then
	echo "IP Forwarding enabled"
	echo "1" > /proc/sys/net/ipv4/ip_forward
fi

if [ "$DUMP_PACKETS" = "yes" ]; then
	tcpdump -i $SLIP_IFACE -X -n -vvv -s 0
fi
