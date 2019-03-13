#!/bin/sh

# Configurable stuff:

TTY=/dev/ttyUSB0       # serial port to use
BAUD=9600              # must match compiled-in value in rs232dev.c
SLATTACH=a8_slattach   # slattach binary (possibly patched)
SLIP_IFACE=sl0         # probably no need to change this
LOCAL_IP=192.168.0.1   # SLIP IP address for Linux host
REMOTE_IP=192.168.0.2  # SLIP IP address for Atari host
IP_FORWARD=yes         # Route packets for the Atari?
IP_MASQUERADE=yes      # NAT for the Atari?
MASQ_IFACE=eth1        # if NATing, our main (LAM or internet) interface
DUMP_PACKETS=yes       # Run tcpdump on sl0 interface?

# DELAY may not be needed on all systems...

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
