#!/bin/sh

# Configurable stuff:

# Name of the disk image (default: command line arg, or fujichat.atr)
ATR_IMAGE=${1-fujichat.atr}

TTY=/dev/ttyS0         # serial port to use
BAUD=4800              # must match FujiChat conf

# This setting is only needed if you're using an AtariMax (Steve Tucker)
# auto-sensing SIO2PC as your serial device.
TUCKER_SIO2PC_HACK="yes"

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

slirp "tty $TTY" "mru 576" "mtu 576" "baudrate $BAUD" &

if [ "$TUCKER_SIO2PC_HACK" = "yes" ]; then
	sleep 1
	./clear_rts
fi
