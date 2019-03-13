
; Third segment is the run address

;.ifdef FUJICHAT
;INITAD = $02E2
; .word INITAD
; .word INITAD+1
; .word dosini_entry_point
;.else
 .word RUNAD
 .word RUNAD+1
 .word dosini_entry_point
;.endif
