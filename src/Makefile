# Copyright (c) 2001, Adam Dunkels.
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
# 3. The name of the author may not be used to endorse or promote
#    products derived from this software without specific prior
#    written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS
# OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
# DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
# GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
# INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
# WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
# NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
# SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
# This file is part of the uIP TCP/IP stack.
#
# $Id: Makefile,v 1.13 2006/06/11 21:55:03 adam Exp $
#

all: uip fujiconf.xex aexec.xex fujimenu.xex makeauto.xex about.xex loadmkau.xex loadmenu.xex keybuf.o clear_rts

CC     = cl65
AR     = ar65
APPS   = telnet resolv
CFLAGS = -t atari -I../uip/uip -I. -O

-include Makefile.include

uip: $(addprefix $(OBJECTDIR)/, fujichat.o common.o commands.o rs232dev.o clock-arch.o) keybuf.o apps.a uip.a fujiput.s

#fujichat.o: common.h features.h fujichat.h # no worky?

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

#loadchat.xex: loadchat.c fujichat.h
#	$(CC) $(CFLAGS) -o loadchat.xex loadchat.c

#loadchat.xex: loadchat.dasm
#dasm loadchat.dasm -f3 -oloadchat.xex

makeauto.xex: makeauto.c
	$(CC) $(CFLAGS) -o makeauto.xex makeauto.c common.o

loadmenu.xex: loadmenu.dasm
	dasm loadmenu.dasm -f3 -oloadmenu.xex

loadmkau.xex: loadmkau.dasm
	dasm loadmkau.dasm -f3 -oloadmkau.xex

aexec.xex: aexec.dasm
	dasm aexec.dasm -f3 -oaexec.xex

test: newdisk
	atariserver fujitest.atr
	sudo sh ./start_slip.sh

disk: newdisk

newdisk: all
	sh mkdisk.sh

old_disk: uip
	cat rvert.com uip > autorun.sys
	axe -w autorun.sys uiptest.atr
	#xterm -e atariserver uiptest.atr

clean:
	rm -fr *.xex obj/ *.o *~ *core uip $(OBJECTDIR) *.a
	mkdir obj

keybuftest.xex: keybuftest.c keybuf.o
	$(CC) $(CFLAGS) -o keybuftest.xex keybuftest.c keybuf.o

# native, not Atari!
clear_rts: clear_rts.c
	cc -o clear_rts clear_rts.c
