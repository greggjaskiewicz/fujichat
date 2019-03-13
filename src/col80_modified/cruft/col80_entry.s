; ----------------------------------------------------------------------------
; The OS jumps here on warmstart (also, this is the run address in our
; binary load file)

dosini_entry_point:
 .ifndef FUJICHAT
        nop
        nop
        nop
 .endif

main_entry_point:
        jsr     col80_init
 .ifndef FUJICHAT
        lda     CONSOL
        and     #$04
        beq     no_e_handler
 .endif
        lda     #$0C
        sta     ICCOM
        ldx     #$00
        jsr     CIOV
        lda     #$58
        sta     font_ptr_lo
        lda     #$03
        sta     ICCOM
        lda     #font_ptr_lo
        sta     ICBAL
        lda     #$00
        sta     ICBAH
        ldx     #$00
        jsr     CIOV
        ldy     #$07
        lda     #<col80_vector_tab
        sta     HATABS,y
        lda     #>col80_vector_tab
        iny
        sta     HATABS,y
no_e_handler:
        lda     #<START_ADDRESS
        sta     MEMTOP
        lda     #>START_ADDRESS
        sta     MEMTOP+1
 .ifdef FUJICHAT
        ldy #$01
		  rts
 .else
        jmp     return_success
 .endif

; ----------------------------------------------------------------------------
; (when does this actually get called? da65 can't find any references
; to it, and it's not a run or init address in the binary load file)
 .ifndef FUJICHAT
        lda     #<dosini_entry_point
        sta     DOSINI
        lda     #>dosini_entry_point
        sta     DOSINI+1
        jmp     main_entry_point
 .endif

