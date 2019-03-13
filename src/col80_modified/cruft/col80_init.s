; ----------------------------------------------------------------------------
; Initialization callback. The OS will call this on coldstart (or would do,
; if the driver were in ROM), and also on warmstart (because we stole the
; DOSINI vector).
; This routine is also the first thing that gets called by the mainline
; init code. Its job is to install COL80 in the handler table at HATABS.
; Actually the handler is first installed as X:, then the main init code
; fixes this up to E: unless the user is holding down SELECT. This allows
; the user to toggle between the 40-column ROM E: and COL80 without doing
; a full reboot. No idea if this was a documented feature or something the
; author used for development/debugging.

col80_init:
        ldy     #$00

next_hatab_slot:
        lda     HATABS,y
        beq     register_x_handler
        iny
        iny
        iny
        cpy     #$20
        bcc     next_hatab_slot
        jmp     return_success

register_x_handler:
        lda     #$58
        sta     HATABS,y
        lda     #<col80_vector_tab
        iny
        sta     HATABS,y
        lda     #>col80_vector_tab
        iny
        sta     HATABS,y
        jmp     return_success
