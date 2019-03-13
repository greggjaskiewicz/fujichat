# source this file to set up your environment for a snapshot of cc65
# installed in a nonstandard place.

CC65_PREFIX=/home/urchlay/cc65_snap

PATH=$CC65_PREFIX/bin:$PATH
CC65_INC=$CC65_PREFIX/lib/cc65/include
CC65_LIB=$CC65_PREFIX/lib/cc65/lib

export PATH CC65_INC CC65_LIB
