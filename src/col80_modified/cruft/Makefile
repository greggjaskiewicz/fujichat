
SRCFILES=col80_dosini_seg.s col80_header_seg.s col80_main.s \
         col80_runad_seg.s col80_startaddr.s

all: cc65_hack
	binload -h col80.xex

cc65_hack: $(SRCFILES) col80.s
	ca65 -t atari -DFUJICHAT col80.s
	ld65 -C col80_cc65_hack.cfg -o col80_main.xex col80.o
	ca65 -t atari -DFUJICHAT col80_startup.s
	ld65 -o col80_startup.xex col80_startup.o
	cat col80_main.xex col80_startup.xex > col80.xex


col80.xex:
	$(MAKE) dasm_build || $(MAKE) ca65_build || $(MAKE) atasm_build

dasm_build: $(SRCFILES) col80.dasm
	dasm col80.dasm -f3 -ocol80.xex

ca65_build: $(SRCFILES) col80.s
	ca65 -t atari col80.s
	ld65 -t atari -o col80.xex col80.o

atasm_build: $(SRCFILES) col80.atasm
	perl dasm2atasm col80_header_seg.s col80_header_seg.atasm
	perl dasm2atasm col80_main.s col80_main.atasm
	atasm -r -ocol80.xex col80.atasm

check: col80.xex
	@if cmp col80.xex col80_orig.xex; then \
		echo "OK: New binary is identical to original" ;\
	else \
		echo "BAD: New binary differs from original" ;\
	fi

clean:
	rm -f *.o col80_header_seg.atasm col80_main.atasm col80.xex

test:
	cp dos_20s.atr test.atr
	cp col80.xex autorun.sys
	axe -w autorun.sys test.atr
	atari800 -nobasic test.atr
