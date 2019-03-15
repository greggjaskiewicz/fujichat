# This file is part of the FujiChat Atari 8-bit IRC client.
# Copyright (c) 2019, B. Watson.
# All rights reserved. See doc/LICENSE.txt for legalese.

CC     = cl65
AR     = ar65
APPS   = telnet resolv
CFLAGS = -t atari -I../uip/uip -I. -O

# Default target: build all the binaries.
all: fujichat.xex fujiconf.xex aexec.xex fujimenu.xex makeauto.xex about.xex loadmkau.xex loadmenu.xex keybuf.o

# The uIP Makefile:
-include Makefile.include

uip: $(addprefix $(OBJECTDIR)/, fujichat.o common.o commands.o rs232dev.o clock-arch.o) keybuf.o apps.a uip.a fujiput.s

fujichat.xex: uip
	mv uip fujichat.xex

keybuf.o: keybuf.s
	ca65 -t atari -l -o keybuf.o keybuf.s

common.o: common.c common.h
	$(CC) $(CFLAGS) -c -o common.o common.c

fujiconf.xex: fujiconf.c fujichat.h common.o
	$(CC) $(CFLAGS) -o fujiconf.xex fujiconf.c common.o obj/uiplib.o

fujimenu.xex: fujimenu.c fujichat.h aexec.xex common.o
	$(CC) $(CFLAGS) -o fujimenu.xex fujimenu.c common.o

about.xex: about.c fujichat.h aexec.xex common.o
	$(CC) $(CFLAGS) -o about.xex about.c common.o

makeauto.xex: makeauto.c
	$(CC) $(CFLAGS) -o makeauto.xex makeauto.c common.o

loadmenu.xex: loadmenu.dasm
	dasm loadmenu.dasm -f3 -oloadmenu.xex

loadmkau.xex: loadmkau.dasm
	dasm loadmkau.dasm -f3 -oloadmkau.xex

aexec.xex: aexec.dasm
	dasm aexec.dasm -f3 -oaexec.xex

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
	cc -o clear_rts clear_rts.c
