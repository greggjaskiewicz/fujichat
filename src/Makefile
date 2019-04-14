# This file is part of the FujiChat Atari 8-bit IRC client.
# Copyright (c) 2019, B. Watson.
# All rights reserved. See doc/LICENSE.txt for legalese.

# If we're working from a git tree, embed the revision and date
# into the binary.
GITREV=$(shell sh ./gitrev)

CC     = cl65
AR     = ar65
APPS   = telnet resolv
CFLAGS = -t atari -I../uip/uip -I. -O -DGITREV=\"$(GITREV)\"

# Default target: build all the binaries.
all: fujichat.xex fujiconf.xex aexec.xex fujimenu.xex makeauto.xex about.xex loadmkau.xex loadmenu.xex keybuf.o

# The uIP Makefile:
-include Makefile.include

uip: $(addprefix $(OBJECTDIR)/, fujichat.o common.o commands.o rs232dev.o clock-arch.o) keybuf.o apps.a uip.a fujiput.s formatip.o

fujichat.xex: uip
	mv uip fujichat.xex

keybuf.o: keybuf.s
	ca65 -t atari -o keybuf.o keybuf.s

common.o: common.c common.h
	$(CC) $(CFLAGS) -c -o common.o common.c

formatip.o: formatip.c formatip.h
	$(CC) $(CFLAGS) -c -o formatip.o formatip.c

fujiconf.xex: fujiconf.c fujichat.h common.o formatip.o
	$(CC) $(CFLAGS) -o fujiconf.xex fujiconf.c common.o formatip.o obj/uiplib.o

fujimenu.xex: fujimenu.c fujichat.h aexec.xex common.o
	$(CC) $(CFLAGS) -o fujimenu.xex fujimenu.c common.o

about.xex: about.c fujichat.h aexec.xex common.o
	$(CC) $(CFLAGS) -o about.xex about.c common.o

makeauto.xex: makeauto.c
	$(CC) $(CFLAGS) -o makeauto.xex makeauto.c common.o

loadmenu.xex: loadxex.s
	cl65 --asm-define MENU -t none -o loadmenu.xex loadxex.s

loadmkau.xex: loadxex.s
	cl65 -t none -o loadmkau.xex loadxex.s

aexec.xex: aexec.s
	cl65 -t none -o aexec.xex aexec.s

test: disk
	atariserver fujitest.atr
	sudo sh ./start_slip.sh

disk: all
	sh mkdisk.sh

clean:
	rm -rf *.xex obj/ *.o *~ *core $(OBJECTDIR) *.a
	mkdir obj

distclean: clean
	rm -rf ../uip/uip/uip.o

## Old test targets, not used, maybe not working any more:

keybuftest.xex: keybuftest.c keybuf.o
	$(CC) $(CFLAGS) -o keybuftest.xex keybuftest.c keybuf.o

# native, not Atari!
clear_rts: clear_rts.c
	gcc -O2 -Wall -o clear_rts clear_rts.c
