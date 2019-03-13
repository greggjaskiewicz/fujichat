
all: col80_hacked.xex

col80_hacked.xex: col80_hacked.dasm
	dasm col80_hacked.dasm -ocol80_hacked.xex -f3
	binload -h col80_hacked.xex

clean:
	rm -f col80_hacked.xex

test:
	cp dos_20s.atr test.atr
	cp col80_hacked.xex autorun.sys
	axe -w autorun.sys test.atr
	atari800 -nobasic test.atr
