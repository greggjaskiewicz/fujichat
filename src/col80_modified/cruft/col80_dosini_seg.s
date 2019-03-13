
; Second segment of the file loads at $0C (aka DOSINI), and just contains
; the address of dosini_entry_point

 .ifndef FUJICHAT
 .word $FFFF ; unnecessary, though the original file had it
 .endif

 .word $000C ; DOSINI
 .word $000D
 .word dosini_entry_point

