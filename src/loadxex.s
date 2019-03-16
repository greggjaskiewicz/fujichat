
; Load FUJIMENU.COM

; This will be part of AUTORUN.SYS on the disk. Without it,
; I'd have to make AUTORUN.SYS a copy of FUJIMENU.COM with
; the serial driver prepended.

RUNAD = $02e0
atari_exec = $0600
main = $2e00 ; same place as cc65

; segment header
 .org main-6
 .word $FFFF
 .word main
 .word endmain-1

 .org main

; If we do this, pressing Reset give us DOS. I want a reboot,
; so leave it out.
;; ldx #1
;; stx BOOTQ
;; dex
;; stx COLDST

; Not much to this...
 lda #<menu_filename
 ldx #>menu_filename
 jmp atari_exec

menu_filename:
 .ifdef MENU
  .byte "D:FUJIMENU.COM", 0
 .else
  .byte "D:MAKEAUTO.COM", 0
 .endif

endmain:

; segment header
 .word RUNAD
 .word RUNAD+1
 .word main

