
COL80 is a software 80-column driver for the Atari 8-bit computer. It
uses GRAPHICS 8 with 4x8 pixel character cells, and replaces the OS
ROM's E: handler.

The file is found in various Atari archives, under various names such
as COL80.COM, COL80E.COM, COL80HND.COM. The original author and date of
publication are unknown.

I've disassembled the binary and labelled/commented the source with
(hopefully) meaningful names. The resulting sources can be reassembled
with the DASM, ca65, or Atasm assemblers, which will result in a binary
that compares as identical to the original.

If you have one of the supported assemblers available on your path,
plus a "make" utility (GNU, BSD, or probably Microsoft's nmake are OK),
you can use the provided Makefile to rebuild the binary (including your
own modified version, if you're into that sort of thing).

File list:

README.txt - you're reading it now

Makefile - the usual

col80_main.s - The actual source code for COL80

col80_dosini_seg.s, col80_header_seg.s, col80_runad_seg.s, and
col80_startaddr.s - Include files, used to build the multi-segment Atari
binary load format object file.

col80.s, col80.dasm, col80.atasm - Top-level wrappers for the various
assemblers, which include the other files in the proper order and using
the proper syntax for the assembler being used.

Modification Ideas:

Implement the missing control character actions. COL80 only does EOL and
the clear-screen code (125), and the others (arrows, delete/insert/bell)
are just printed in their graphical form.

The original COL80 loads the driver code at $7A00, so it'll be compatible
with BASIC, or other cartridge software. I've built a version org'ed at
$9A00, which works great with disk-only software and gives an extra 8K
of available RAM (change START_ADDR in col80_startaddr.s).

It should be possible to use 4x7 or 4x6 character cells instead of
4x8. The font would of course need to be redesigned, and the characters
would be even smaller than they are now, but this would give you 27 or
32 rows of text on screen (or more, if you expand the display by a few
scanlines). With a good green or amber monitor and luma-only output,
this could be usable.

Instead of inverse video for characters 128-255, could do an expanded
international character set (ISO Latin-1). Add a UTF-8 parser and you've
got Unicode on the Atari!

Add a VT100/ANSI escape-sequence parser. Could render actual underlined
characters, and bold as inverse video. ANSI color codes we can't easily
do, but could at least strip them out.

Squeeze the driver down to save RAM. Use the standard E: buffer in page 5,
move the code up so it ends just before the GR.8 display list, eliminate
the code that installs the handler as X: and checks for the SELECT key
being held down... get rid of the margin beep.  Use RMARGN in zero page
instead of right_margin at $7C00, move the other COL80 variables to
page zero. Eliminate the lookup tables, if they can be replaced with
code that takes up less space and calculates the values on the fly.
The current driver code is 3 pages long; it might be possible to squish
it into 2 pages... like, say, page 6 and the cassette buffer, or make it
auto-relocate itself to MEMLO like Bob-Verter does. Using a 4x6 or 4x7
font shrinks the font table, too... another thing to do would be to get
rid of the clear_screen routine (replace with a call to init_graphics_8)

For XL/XE machines, turn COL80 into an OS patch. For modified 400/800
machines with RAM at $C000-CFFF, move COL80 there. For 130XEs, use an
extended RAM bank for the driver, and another bank for the screen RAM
(separate ANTIC/CPU access mode, won't work on most upgraded 800XLs). Just
keep a tiny stub driver in main RAM, that switches in the driver's bank
and jumps to it.

Make a COL64 driver (like the SpartaDOS X CON64.SYS). Use 5x8 characters
for 64 columns (or 5x6 for 64x32). Probably this would count more as
a rewrite than a modification. The font would have to be stored one
character per 8 bytes (take up twice as much space), and lots of shifting
would have to happen when writing to the screen (slow)... Could also
do 56 columns (7 pixel wide), and actually use the ROM font (just cut
off the high bit, and for 56x27 also cut off the bottom scanline).

