#!/bin/sh

ATR_IMAGE=${1-dumb.atr}
TTY=/dev/ttyS0

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
strace -o/dev/null agetty -L $TTY 4800 vt100 &
$DELAY
./clear_rts
