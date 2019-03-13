#!/bin/sh

set -e

rm -rf disk
mkdir disk
cp dos_20s.atr disk/fujichat.atr
cp uip fujichat.xex
cp atari850.ser disk/atari850.ser
cp bobvert.com disk/bobvert.ser
cp prconn.ser disk/prconn.ser

if [ -f about.txt ]; then
	perl -pe 's/\n/\x9b/' < about.txt > disk/about.txt
fi

cd disk
for i in about loadmenu fujimenu makeauto fujiconf fujichat; do
	cat ../aexec.xex ../$i.xex > $i.com
	axe -w $i.com fujichat.atr
done

#cat ../bobvert.com fujimenu.com > autorun.sys
#cat fujimenu.com > autorun.sys

cat ../aexec.xex ../loadmkau.xex > autorun.sys
axe -w autorun.sys fujichat.atr

axe -w atari850.ser fujichat.atr
axe -w bobvert.ser fujichat.atr
axe -w prconn.ser fujichat.atr

if [ -f about.txt ]; then
	axe -w about.txt fujichat.atr
fi

cp fujichat.atr ..

# build my own test disk so I don't have to keep going thru the
# serial port and config every time
if [ "`whoami`" = "urchlay" ]; then
	cat bobvert.ser loadmenu.com > autorun.sys
	cp ../fujichat.cfg .
	cp fujichat.atr fujitest.atr
	axe -w autorun.sys fujitest.atr
	axe -w fujichat.cfg fujitest.atr
	mv fujitest.atr ..
fi
