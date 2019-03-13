
; ca65 wrapper for building col80

 .setcpu "6502"

 .segment "EXEHDR"
 .include "col80_startaddr.s"
 .include "col80_header_seg.s"

 .segment "CODE"
 .include "col80_main.s"

 .ifndef FUJICHAT
 .include "col80_dosini_seg.s"

 .segment "AUTOSTRT"
 .include "col80_runad_seg.s"
 .endif

 .segment "ZPSAVE"
 ; nothing to see here, just shutting up ld65's warning
