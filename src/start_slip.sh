#!/bin/sh

# Configurable stuff. You only ever need to change the IP addresses
# if your LAN already uses 192.168.0.0/24.

TTY=/dev/ttyUSB0       # serial port to use
BAUD=9600              # must match baud rate in fujichat.cfg on the Atari
LOCAL_IP=192.168.0.1   # SLIP IP address for Linux ("peer") host
REMOTE_IP=192.168.0.2  # SLIP IP address for Atari host

# You want these two enabled, unless you're running an IRC server locally,
# and don't want to connect FujiChat to the real Internet:
IP_FORWARD=yes         # Route packets for the Atari? (you want this, unless
                       # you're running an IRC server locally)
IP_MASQUERADE=yes      # NAT for the Atari?
DUMP_PACKETS=yes       # Run tcpdump on sl0 interface?

# Optional. Autodetected, if you leave these commented out (recommended).

#SLATTACH=a8_slattach  # slattach binary (possibly patched). if not set,
                       # search $PATH for a8_slattach, then fall back to
                       # regular slattach.

#MASQ_IFACE=eth1       # if NATing, our main (LAM or internet) interface
                       # (if not set, detect automagically)

#SLIP_IFACE=sl0        # don't change this unless you know exactly why
                       # you need to!

# DELAY may not be needed on all systems...

# No delay:
#DELAY="true"

# 1 second:
#DELAY="sleep 1"

# 1/4 second (may not work on old Linux installs):
DELAY="usleep 250000"

# End of config section, start of code:

# Just in case:
/sbin/modprobe slip 2>/dev/null

if [ "$SLATTACH" = "" ]; then
	SLATTACH="$( which a8_slattach 2>/dev/null )"
	if [ "$SLATTACH" = "" ]; then
		SLATTACH="$( which slattach 2>/dev/null )"
	fi
fi
echo "Using SLATTACH=$SLATTACH"

# Warning: this assumes nothing else is using SLIP. Probably a safe
# assumption in the 21st century.
SLIP_IFACE="${SLIP_IFACE:-sl0}"
ifconfig $SLIP_IFACE down 2>/dev/null
killall $SLATTACH 2>/dev/null
fuser -k $TTY 2>/dev/null # make sure nobody's using the port...
$DELAY
fuser -k -9 $TTY 2>/dev/null # Just in case...
$DELAY

echo "Starting SLIP on $SLIP_IFACE, local $LOCAL_IP, remote $REMOTE_IP"
$SLATTACH -L -p slip -s $BAUD $TTY &
$DELAY
ifconfig $SLIP_IFACE $LOCAL_IP mtu 576
ifconfig $SLIP_IFACE $LOCAL_IP pointopoint $REMOTE_IP

if [ "$IP_MASQUERADE" = "yes" ]; then
	if [ "$MASQ_IFACE" = "" ]; then
		MASQ_IFACE="$(/sbin/ip route show 2>/dev/null | sed -n 's,^default.*dev *\([^ ]*\).*$,\1,p')"
		if [ "$MASQ_IFACE" = "" ]; then
			MASQ_IFACE="$(/sbin/route -n | grep '^0.0.0.0' | rev | cut -d' ' -f1 |rev)"
			if [ "$MASQ_IFACE" = "" ]; then
				echo "Can't autodetect default interface, guessing eth0"
				MASQ_IFACE="eth0"
			fi
		fi
	fi
	echo "IP Masquerading enabled on device $MASQ_IFACE"
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
