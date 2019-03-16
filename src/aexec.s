
; Load an Atari DOS executable, including support for init and run vectors
; Parameters: nul-terminated filename pointer in A/X (lo/hi)
; Return value: 0 for success, nonzero on error, never returns if exe has
; a run address.

; This is for the Atari, but it gets built with "-t none" to
; avoid ld65 adding the C runtime startup code. Also, there is
; NO run address in this file.

; "Link" to your program with "cat aexec.xex yourprog.xex > newprog.xex"

; int __fastcall__ (*atari_exec_p)(char *) = (int __fastcall__ (*)(char *))0x600;
; #define atari_exec(x) ((*atari_exec_p)(x))
; then call with: atari_exec("D:PROGRAM.XEX");

; If the file can't be opened for whatever reason, atari_exec() returns a
; nonzero result (the Atari error number).

; If the file loads OK and has a run address, atari_exec() never returns
; (runs the loaded program, then exits to DOS if/when the program exits).
; This means there's no memory conflict between caller and callee.

; If there was no run address, atari_exec returns 0 to the caller.
; The caller is responsible for making sure the loaded program doesn't
; step on the memory used by the caller or atari_exec itself!

; If the file is openable but invalid (not a XEX, or truncated), current
; atari_exec() implementation hangs the machine with a red screen, since
; there's no way to know whether the partially-loaded program overwrote
; the caller...


 .include "atari.inc"

tmp = $d4 ; aka FR0
loadaddr = $d6

main = $0600

 .org main-6
 .word $FFFF
 .word main
 .word endmain-1

 .org main
 pha
 txa
 pha
 jsr fclose
 pla
 tax
 pla

 sta ICBAL+16
 stx ICBAH+16
 sta tmp
 stx tmp+1
 ldy #0

; do an OPEN #1,4,0,$filename

; get length of filename
fnameloop:
 lda (tmp),y
 beq fndone
 iny
 bne fnameloop

 ; set IOCB #1 buffer addr, buffer len, etc
fndone:
 sty ICBLL+$10
 lda #0
 sta ICBLH+$10
 sta ICAX2+$10

 ; init these to 0 so we can tell if they change
 sta RUNAD
 sta RUNAD+1
 sta INITAD
 sta INITAD+1

 ldx #4
 stx ICAX1+$10
 dex
 stx ICCOM+$10 ; cmd #3 = OPEN

 ldx #$10
 jsr CIOV ; do the OPEN
 bpl readheader

 tya ; CIO returns error code in Y reg
 ldx #0
 rts

fail:
 lda #$48
 sta 710
hang: bne hang

readheader:
; read 2 bytes into local buffer
 jsr read2bytes
 bpl ok
 cpy #136 ; EOF
 bne fail
 beq close_file ; if at EOF, close the file

; if they're $ffff, try again
ok:
 lda tmp
 tax
 and tmp+1
 cmp #$ff
 beq readheader

; store those 2 bytes in loadaddr
 lda tmp+1
 sta loadaddr+1
 stx loadaddr

; read 2 more bytes into local buffer (end addr)
 jsr read2bytes
 bmi fail
; subtract loadaddr from end addr, add 1, to get length
; store length into IOCB
 lda loadaddr
 sta ICBAL+$10
 lda loadaddr+1
 sta ICBAH+$10

 sec
 lda tmp
 sbc loadaddr
 sta ICBLL+$10
 lda tmp+1
 sbc loadaddr+1
 sta ICBLH+$10
 inc ICBLL+$10
 bne noinc
 inc ICBLH+$10
noinc:
 jsr read_segment
 bmi fail

; if INITAD modified, JSR there
 lda INITAD
 ora INITAD+1
 beq readheader

 jsr do_init

 lda #0
 sta INITAD
 sta INITAD+1
 beq readheader ; branch always

do_init:
 jmp (INITAD)

close_file:
 jsr fclose

; JSR through RUNAD if it's not zero
 lda RUNAD
 ora RUNAD+1
 beq norun

 jsr do_run
 jmp (DOSVEC) ; does not return

; If there was no run address, we were probably loading an R: driver, so
; our caller is safe to return to
norun:
 lda #0 ; return 0
 tax
 rts

read2bytes:
 lda #tmp
 sta ICBAL+$10
 lda #2
 sta ICBLL+$10
 lda #0
 sta ICBAH+$10
 sta ICBLH+$10
read_segment:
 lda #GETCHR
 sta ICCOM+$10
 ldx #$10
 jmp CIOV

do_run:
 jmp (RUNAD)

fclose:
 lda #CLOSE
 sta ICCOM+$10
 ldx #$10
 jmp CIOV

endmain:
