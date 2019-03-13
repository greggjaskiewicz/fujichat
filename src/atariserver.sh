#!/bin/sh

# Configurable stuff:

# Name of the disk image (default: command line arg, or fujichat.atr)
ATR_IMAGE=${1-fujichat.atr}

TTY=/dev/ttyS0         # serial port to use

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
