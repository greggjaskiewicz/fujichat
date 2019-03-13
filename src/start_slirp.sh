#!/bin/sh

# Configurable stuff:

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

slirp "tty $TTY" "mru 576" "mtu 576" "baudrate $BAUD" &

if [ "$TUCKER_SIO2PC_HACK" = "yes" ]; then
	sleep 1
	./clear_rts
fi
