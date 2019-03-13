; THIS IS A MODIFIED VERSION, for use with FujiChat

; COL80.COM, aka COL80E.COM, aka COL80HND.COM
; (and probably several other names)

; Original author unknown
; License unknown
; Disassembly and comments by Urchlay

; This is a widely-distributed software 80-column driver for the Atari
; 8-bit computers. It replaces the OS's E: driver, and uses GRAPHICS 8
; for display, with 4x8 pixel character cells.

; Disassembly was done with da65, with many iterations of "edit the
; .info file, disassemble again", and the results were tweaked by hand
; into something assemblable by dasm (and fairly compatible with other
; assemblers).


 .include "col80_include.s"

; START_ADDRESS is defined in col80_startaddr.s
 .org START_ADDRESS

; ----------------------------------------------------------------------------
; Start of COL80. The font is stored in packed form. Each group of 8 bytes
; defines two glyphs: the upper 4 bits of the 8 bytes, taken together,
; define the bitmap for the first glyph, and the lower 4 bits are the second.
; Note that the bits that make up a single character are spread across 8
; bytes, so it's hard to visualize these even if you're used to reading hex
; dumps.

; The first 2 characters look like:

; .... .O.. ; $04
; .... .O.. ; $04
; O.O. .O.. ; $A4
; OOO. .O.. ; $E4
; OOO. .OOO ; $E7
; .O.. .O.. ; $44
; .... .O.. ; $04
; .... .O.. ; $04

; These are the ATASCII heart symbol (character code 0) and the ATASCII
; control-A line-drawing symbol (code 1).

; Note: unlike the ROM font, this font is stored in ATASCII order instead
; of the standard Atari character order imposed by the hardware. Like
; the ROM font, inverse characters are not stored here (the bitmaps get
; inverted by the driver)

font_data:
 .ifdef FUJICHAT
  .include "new_font.s"
 .else
        ; Low ATASCII graphics symbols (code 0-31)
        .byte   $04,$04,$A4,$E4,$E7,$44,$04,$04
        .byte   $14,$14,$14,$14,$1C,$10,$10,$10
        .byte   $40,$40,$40,$40,$CC,$44,$44,$44
        .byte   $18,$18,$24,$24,$42,$42,$81,$81
        .byte   $10,$10,$30,$30,$73,$73,$F3,$F3
        .byte   $83,$83,$C3,$C3,$E0,$E0,$F0,$F0
        .byte   $CF,$CF,$C0,$C0,$00,$00,$00,$00
        .byte   $00,$00,$00,$00,$0C,$0C,$FC,$FC
        .byte   $00,$00,$00,$40,$A7,$44,$E4,$04
        .byte   $04,$04,$04,$04,$FF,$04,$04,$04
        .byte   $00,$00,$60,$F0,$FF,$6F,$0F,$0F
        .byte   $80,$80,$80,$80,$8F,$84,$84,$84
        .byte   $4C,$4C,$4C,$4C,$FC,$0C,$0C,$0C
        .byte   $40,$4C,$48,$4C,$78,$0C,$06,$00
        .byte   $00,$44,$E4,$44,$4E,$44,$00,$00
        .byte   $00,$24,$42,$FF,$42,$24,$00,$00

        ; Space ! " # etc (codes 32-63)
        .byte   $00,$04,$04,$04,$04,$00,$04,$00
        .byte   $00,$A0,$AA,$AE,$0A,$0E,$0A,$00
        .byte   $00,$40,$68,$82,$44,$28,$C2,$40
        .byte   $00,$C4,$64,$E4,$60,$C0,$40,$00
        .byte   $00,$44,$82,$82,$82,$82,$82,$44
        .byte   $00,$04,$A4,$4E,$E4,$44,$A0,$00
        .byte   $00,$00,$00,$0E,$00,$40,$40,$80
        .byte   $00,$02,$02,$04,$04,$08,$48,$00
        .byte   $00,$E4,$AC,$A4,$A4,$A4,$EE,$00
        .byte   $00,$EE,$22,$22,$EE,$82,$EE,$00
        .byte   $00,$AE,$A8,$AE,$E2,$22,$2E,$00
        .byte   $00,$EE,$82,$E2,$A4,$A4,$E4,$00
        .byte   $00,$EE,$AA,$EA,$AE,$A2,$EE,$00
        .byte   $00,$00,$00,$44,$00,$44,$04,$08
        .byte   $00,$20,$4E,$80,$4E,$20,$00,$00
        .byte   $00,$8C,$42,$22,$44,$80,$04,$00

        ; @ A B C etc (codes 64-95)
        .byte   $00,$6E,$9A,$BA,$BE,$8A,$6A,$00
        .byte   $00,$C6,$A8,$C8,$A8,$A8,$C6,$00
        .byte   $00,$CE,$A8,$AC,$A8,$A8,$CE,$00
        .byte   $00,$E6,$88,$C8,$8A,$8A,$86,$00
        .byte   $00,$AE,$A4,$E4,$A4,$A4,$AE,$00
        .byte   $00,$2A,$2A,$2C,$2A,$2A,$CA,$00
        .byte   $00,$8A,$8E,$8E,$8A,$8A,$EA,$00
        .byte   $00,$C4,$AA,$AA,$AA,$AA,$A4,$00
        .byte   $00,$EE,$AA,$EA,$8A,$8A,$8E,$03
        .byte   $00,$C6,$A8,$AC,$C2,$A2,$AC,$00
        .byte   $00,$EA,$4A,$4A,$4A,$4A,$4E,$00
        .byte   $00,$AA,$AA,$AA,$AE,$AE,$4A,$00
        .byte   $00,$AA,$4A,$4E,$44,$44,$A4,$00
        .byte   $00,$EE,$28,$48,$88,$88,$E8,$0E
        .byte   $00,$8E,$82,$42,$42,$22,$22,$0E
        .byte   $00,$00,$40,$A0,$00,$00,$00,$0F

        ; diamond, lowercase letters, control codes (codes 96-127)
        .byte   $00,$00,$00,$46,$E2,$4E,$0E,$00
        .byte   $00,$80,$80,$C6,$A8,$A8,$C6,$00
        .byte   $00,$20,$20,$6E,$AE,$A8,$6E,$00
        .byte   $00,$00,$C0,$86,$CA,$8E,$82,$0C
        .byte   $00,$80,$84,$80,$C4,$A4,$A4,$00
        .byte   $00,$08,$28,$0A,$2C,$2A,$2A,$C0
        .byte   $00,$40,$40,$4A,$4E,$4A,$4A,$00
        .byte   $00,$00,$00,$CE,$AA,$AA,$AE,$00
        .byte   $00,$00,$00,$C6,$AA,$C6,$82,$82
        .byte   $00,$00,$00,$6E,$88,$86,$8E,$00
        .byte   $00,$00,$40,$EA,$4A,$4A,$6E,$00
        .byte   $00,$00,$00,$AA,$AA,$AE,$4A,$00
        .byte   $00,$00,$00,$AA,$4A,$A6,$A2,$0C
        .byte   $00,$00,$04,$EE,$4E,$84,$EE,$00
        .byte   $40,$4E,$4C,$4E,$4A,$42,$42,$40
        .byte   $00,$28,$6C,$EE,$6C,$28,$00,$00
 .endif

right_margin:
        ; Default value is 79 decimal. Unsure why the author didn't use RMARGN at $53
        .byte   $4F

; ----------------------------------------------------------------------------
; Start of COL80 code.

; Callback for CIO OPEN command.

col80_open:
        jsr     init_graphics_8
        lda     #$00
        sta     ROWCRS
        sta     COLCRS
 .ifndef FUJICHAT
        nop
        nop
 .endif
        sta     BUFCNT
        lda     #$4F
        sta     right_margin
        rts

; ----------------------------------------------------------------------------
; Assembly version of GRAPHICS 8+16 command.

init_graphics_8:
        lda     #$08
        sta     ICAX2Z
        lda     #$0C
        sta     ICAX1Z
        jsr     open_s_dev

        ; Set COL80's default colors
        lda     #$08
        sta     COLOR2
 .ifndef FUJICHAT
        nop
        nop
        nop
 .endif
        lda     #$00
        sta     COLOR1

        ; Protect ourselves from BASIC and the OS
        lda     #<START_ADDRESS
        sta     MEMTOP
        lda     #>START_ADDRESS
        sta     MEMTOP+1
        rts

; ----------------------------------------------------------------------------
; Call the OPEN vector for the S: device, using the ROM vector table
; at $E410. The table stores address-minus-one of each routine, which is
; meant to actually be called via the RTS instruction (standard 6502
; technique, but confusing the first time you encounter it)

open_s_dev:
        lda     s_dev_open_hi
        pha
        lda     s_dev_open_lo
        pha
        rts

; ----------------------------------------------------------------------------
; Callback for CIO CLOSE command. Note that the routine does nothing, really
; (the OS will mark the E: device as being closed, but COL80 doesn't do any
; cleanup).
; The SPECIAL and GET STATUS callbacks in col80_vector_tab also point here.

col80_close:
        jmp return_success

; ----------------------------------------------------------------------------
; Callback for the internal put-one-byte, used by the OS to implement the
; CIO PUT RECORD and PUT BYTES commands. This routine's one argument is
; the byte in the accumulator (the character to print).

; First, the routine checks for the cursor control characters it supports.
; COL80 only handles the EOL and clear-screen codes; trying to print
; backspaces, arrows, deletes, inserts, etc just causes their ATASCII
; graphics character to print instead.

col80_putbyte:
        ; EOL (decimal 155)?
        cmp     #$9B
        bne     check_clear
        lda     right_margin
        sta     COLCRS
        jmp     skip_write

check_clear:
 .ifndef FUJICHAT ; save memory by not including clear_screen
                  ; (also, this lets us print the } character)
        ; Clear (decimal 125)?
        cmp     #$7D
        bne     regular_char
        jmp     clear_screen
 .endif

        ; See if this is an inverse video char (code >= 128)
regular_char:
        tax
        bpl     not_inverse
        lda     #$FF
        sta     inverse_mask
        bne     skip_ninv

not_inverse:
        lda     #$00
        sta     inverse_mask

skip_ninv:
        txa
        and     #$7F
 .ifdef FUJICHAT ; mask out low ASCII
        sec
        sbc #$20
        bcs not_low_ascii
        jmp return_success
not_low_ascii:
 .endif
        sta     TMPCHR
        lda     DINDEX
        cmp     #$08
        beq     graphics_ok
        ; If we're not in GRAPHICS 8 mode, reinitialize ourselves
        jsr     col80_open

graphics_ok:
        ; Call the routines that actually print the character
        jsr     setup_font_ptr
        jsr     setup_screen_ptr
        jsr     write_font_data

skip_write:
        ; Move the cursor 1 space to the right. This will
        ; advance us to the next line if we're at the margin,
        ; and scroll the screen if needed
        jsr     advance_cursor

check_ssflag:
        ; The OS keyboard interrupt handler will toggle SSFLAG (start/stop fla
        ; any time the user presses ctrl-1
        lda     SSFLAG
        bne     check_ssflag
        jmp     return_success

; ----------------------------------------------------------------------------
; Scroll the screen up one line (8 scanlines). This has to move almost 8K of
; data, so it's noticeably slower than scrolling the GR.0 text screen.

scroll_screen:
        lda     SAVMSC
        sta     screen_ptr_lo
        clc
        adc     #$40
        ; font_ptr_lo is actually being used here as a second pointer into
        ; screen RAM, instead of its usual use as a pointer into the
        ; font_data table
        sta     font_ptr_lo
        lda     SAVMSC+1
        sta     screen_ptr_hi
        adc     #$01
        sta     font_ptr_hi
        ldx     #$1D
        ldy     #$00

scroll_line_loop:
        lda     (font_ptr_lo),y
        sta     (screen_ptr_lo),y
        dey
        bne     scroll_line_loop
        inc     font_ptr_hi
        inc     screen_ptr_hi
        dex
        bne     scroll_line_loop

blank_bottom_row:
        lda     SAVMSC
        clc
        adc     #$C0
        sta     screen_ptr_lo
        lda     SAVMSC+1
        adc     #$1C
        sta     screen_ptr_hi
        lda     #$00
        tay

blank_loop:
        sta     (screen_ptr_lo),y
        dey
        bne     blank_loop
        inc     screen_ptr_hi
        ldy     #$40

blank_tail:
        sta     (screen_ptr_lo),y
        dey
        bpl     blank_tail
        rts

; ----------------------------------------------------------------------------
; Set up font_ptr_lo/hi to point to the font_data bitmap for the character in
; TMPCHR. Also sets lo_nybble_flag to let the caller know whether the
; bitmap is in the upper or lower 4 bits of the bytes pointed to.

setup_font_ptr:
        lda     #$00
        sta     font_ptr_hi
        sta     lo_nybble_flag
        lda     TMPCHR
        clc
        ror
        bcc     font_hi_nybble
        ldx     #$FF
        stx     lo_nybble_flag

font_hi_nybble:
        clc
        rol
        rol
        rol     font_ptr_hi
        rol
        rol     font_ptr_hi
        adc     #<font_data
        sta     font_ptr_lo
        lda     #>font_data
        adc     font_ptr_hi
        sta     font_ptr_hi
        rts

; ----------------------------------------------------------------------------
; Move the cursor one space to the right (to the next line if at the margin,
; and scroll screen if on the last row)

advance_cursor:
        inc     COLCRS
        lda     right_margin
        cmp     COLCRS
        bcs     same_line
        lda     LMARGN
        sta     COLCRS
        lda     ROWCRS
        ; $17 is 25 decimal, one row below the lowest on the screen
        cmp     #$17
        bcc     no_scroll
        jsr     scroll_screen
        ; Move to row 24 after scrolling
        lda     #$16
        sta     ROWCRS

no_scroll:
        inc     ROWCRS

same_line:
        rts

; ----------------------------------------------------------------------------
; Clear the screen by setting all screen RAM bytes to zero. Slow, but not
; as slow as scrolling.

 .ifndef FUJICHAT
clear_screen:
        lda     SAVMSC
        sta     screen_ptr_lo
        lda     SAVMSC+1
        sta     screen_ptr_hi
        ldy     #$00
        ldx     #$1D
        lda     #$00

cls_loop:
        sta     (screen_ptr_lo),y
        dey
        bne     cls_loop
        inc     screen_ptr_hi
        dex
        bne     cls_loop
        jsr     blank_bottom_row
        lda     LMARGN
        sta     COLCRS
        lda     #$00
        sta     ROWCRS
        ; redundant JMP
        jmp     return_success
 .endif

; ----------------------------------------------------------------------------
; CIO expects the Y register to contain a status code.
; 1 means success (no error). Lots of COL80's routines
; jump here.

return_success:
        ldy     #$01
        rts

; ----------------------------------------------------------------------------
; Set screen_ptr_lo/hi to point to the address of the first byte of graphics
; data at the current cursor position.

setup_screen_ptr:
        ldy     ROWCRS
        lda     SAVMSC
        clc
        adc     row_low_offset_tab,y
        sta     screen_ptr_lo
        lda     SAVMSC+1
        adc     row_high_offset_tab,y
        sta     screen_ptr_hi
        lda     COLCRS
        lsr
        clc
        adc     screen_ptr_lo
        bcc     hi_byte_ok
        inc     screen_ptr_hi

hi_byte_ok:
        sta     screen_ptr_lo
        rts

; ----------------------------------------------------------------------------
; Tables of offsets for setup_screen_ptr, to avoid doing multiplication at
; runtime (the 6502 lacks a MUL instruction, so it's slow...)

row_low_offset_tab:
        .byte   $00,$40,$80,$C0,$00,$40,$80,$C0
        .byte   $00,$40,$80,$C0,$00,$40,$80,$C0
        .byte   $00,$40,$80,$C0,$00,$40,$80,$C0

row_high_offset_tab:
        .byte   $00,$01,$02,$03,$05,$06,$07,$08
        .byte   $0A,$0B,$0C,$0D,$0F,$10,$11,$12
        .byte   $14,$15,$16,$17,$19,$1A,$1B,$1C

; ----------------------------------------------------------------------------
; Copy pixel data from the font table to screen RAM.
; font_ptr_lo/hi must point to the correct character, and screen_ptr_lo/hi
; must point to the correct screen address for the current cursor position.
; This routine has separate execution paths for even- and odd-numbered
; cursor positions, since each byte of screen RAM holds data for two
; adjacent characters (and when printing to one of them, the other needs
; to be left undisturbed!)

write_font_data:
        lda     COLCRS
        clc
        ror
        bcc     write_font_data_even
        ldx     #$00
        ldy     #$00

get_font_nybble_odd:
        lda     (font_ptr_lo),y
        bit     lo_nybble_flag
        bne     lo_nybble_odd
        ; glyph we want is stored in top 4 bits of font byte,
        ; shift it down to the bottom 4 bits
        lsr
        lsr
        lsr
        lsr

lo_nybble_odd:
        eor     inverse_mask
        and     #$0F
        sta     TMPCHR
        ldy     scanline_offset_tab,x
        lda     (screen_ptr_lo),y
        and     #$F0
        ora     TMPCHR
        sta     (screen_ptr_lo),y
        inx
        cpx     #$07
        bne     screen_ptr_ok_odd
        inc     screen_ptr_hi

screen_ptr_ok_odd:
        cpx     #$08
        beq     write_font_done_odd
        txa
        tay
        bne     get_font_nybble_odd

write_font_done_odd:
        rts

; ----------------------------------------------------------------------------
; Write data to even-numbered columns, very similar to the above

write_font_data_even:
        ldx     #$00
        ldy     #$00

get_font_nybble_even:
        lda     (font_ptr_lo),y
        bit     lo_nybble_flag
        beq     hi_nybble_even
        asl
        asl
        asl
        asl

hi_nybble_even:
        eor     inverse_mask
        and     #$F0
        sta     TMPCHR
        ldy     scanline_offset_tab,x
        lda     (screen_ptr_lo),y
        and     #$0F
        ora     TMPCHR
        sta     (screen_ptr_lo),y
        inx
        cpx     #$07
        bne     screen_ptr_ok_even
        inc     screen_ptr_hi

screen_ptr_ok_even:
        cpx     #$08
        beq     write_font_done_even
        txa
        tay
        bne     get_font_nybble_even

write_font_done_even:
        rts

; ----------------------------------------------------------------------------

scanline_offset_tab:
        .byte   $00,$28,$50,$78,$A0,$C8,$F0,$18

; ----------------------------------------------------------------------------
; Callback for the internal get-one-byte, used by the OS to implement the
; CIO GET RECORD and GET BYTES commands. This routine takes no arguments,
; and returns the read byte in the accumulator.

; Internally, COL80 maintains a line buffer. Each time col80_getbyte is
; called, it returns the next character in the buffer. If the buffer's
; empty (or if the last call returned the last character), a new line
; of input is read from the user (and the first character is returned).
; This is exactly how the OS E: device works.

col80_getbyte:
        lda     BUFCNT
        beq     get_line

get_next_byte:
        ldx     line_buffer_index
        lda     line_buffer,x
        dec     BUFCNT
        inc     line_buffer_index
        jmp     return_success

; ----------------------------------------------------------------------------
; Get a line of input from the user, terminated by the Return key.

get_line:
        lda     #$00
        sta     BUFCNT
        sta     line_buffer_index

show_cursor:
 .ifdef FUJICHAT
        lda     #$00
 .else
        lda     #$20
 .endif
        sta     TMPCHR
        lda     #$FF
        sta     inverse_mask
        jsr     setup_font_ptr
        jsr     setup_screen_ptr
        jsr     write_font_data
        jsr     get_keystroke
        cpy     #$01
        beq     keystroke_ok
 .ifdef FUJICHAT
        dey ; yes, we really care about 1-byte optimizations
 .else
        ldy     #$00
 .endif
        sty     line_buffer_index
        sty     BUFCNT

keystroke_ok:
 .ifdef FUJICHAT
        cmp #$20
		  bcc show_cursor ; ignore low ASCII
 .endif
        cmp     #$9B
        bne     check_backs_key
        jmp     return_key_hit

check_backs_key:
        cmp     #$7E
        bne     check_clear_key
        jmp     backs_key_hit

check_clear_key:
        cmp     #$7D
        bne     normal_key_hit
        jmp     clear_key_hit

normal_key_hit:
        ldx     BUFCNT
        bpl     buffer_character
 .ifdef FUJICHAT
        jmp show_cursor
 .else
        jmp     beep
 .endif

buffer_character:
        sta     line_buffer,x
        jsr     col80_putbyte
        inc     BUFCNT
        jmp     show_cursor

return_key_hit:
        jsr     print_space
        lda     #$9B
        ldx     BUFCNT
        sta     line_buffer,x
        inc     BUFCNT
        jsr     col80_putbyte
        jmp     get_next_byte

clear_key_hit:
 .ifndef FUJICHAT
        jsr     clear_screen
 .endif
        lda     #$00
        sta     line_buffer_index
        sta     BUFCNT
        jmp     get_line

backs_key_hit:
        jsr     print_space
        lda     BUFCNT
        beq     backs_key_done
        dec     COLCRS
        lda     COLCRS
        clc
        adc     #$01
        cmp     LMARGN
        bne     backs_same_line
        lda     right_margin
        sta     COLCRS
        dec     ROWCRS

backs_same_line:
        dec     BUFCNT

backs_key_done:
        jmp     show_cursor

; ----------------------------------------------------------------------------
; Ring the margin bell. COL80 doesn't implement the ctrl-2 bell (character
; 253), and instead of using the GTIA keyclick speaker, it uses POKEY to
; make a beep

 .ifndef FUJICHAT
beep:   ldy     #$00
        ldx     #$AF

beep_delay_x:
        stx     AUDF1
        stx     AUDC1

beep_delay_y:
        dey
        bne     beep_delay_y
        dex
        cpx     #$9F
        bne     beep_delay_x
        jmp     show_cursor
 .endif

; ----------------------------------------------------------------------------
; Print a space character at the current cursor position. Does not
; update the cursor position.
print_space:
        lda     #$00
        sta     inverse_mask
 .ifndef FUJICHAT
        lda     #$20
 .endif
        sta     TMPCHR
        jsr     setup_font_ptr
        jsr     setup_screen_ptr
        jsr     write_font_data
        rts

; ----------------------------------------------------------------------------
; Get a keystroke (blocking). Just calls the OS K: get-one-byte routine
; (call by pushing address-minus-one then doing an RTS)
get_keystroke:
        lda     k_dev_get_hi
        pha
        lda     k_dev_get_lo
        pha
        rts

 .ifndef FUJICHAT
 .include "col80_init.s"
 .endif

; ----------------------------------------------------------------------------
; COL80 vector table, in the format required by the OS. Our HATABS entry
; will point to this table, and the OS will call the routines listed here
; via the "call by RTS" method (which is why they're address-minus-one).

; See the entry on HATABS in "Mapping the Atari" or the OS manual.

col80_vector_tab:
        .word   col80_open-1
        .word   col80_close-1
        .word   col80_getbyte-1
        .word   col80_putbyte-1
        .word   col80_close-1
        .word   col80_close-1
 .ifdef FUJICHAT
        .byte 0, 0, 0 ; heh.
 .else
        jmp     col80_init
 .endif

 .ifndef FUJICHAT
 .include "col80_entry.s"
 .endif

; ----------------------------------------------------------------------------
; Various bits of runtime state here. It's unclear to me why the standard
; OS buffer location couldn't have been used instead (normally the top
; half of page 5), or why the other stuff couldn't have been stored in
; zero page, in locations used by the ROM E: handler (thus unused when
; it's replaced with COL80). line_buffer_index needs to be preserved
; across calls to col80_getbyte, but lo_nybble_flag and inverse_mask are
; freshly calculated every time they're used, so they could be almost
; anywhere.

 .ifdef FUJICHAT
 .segment "CODE"
 .endif

lo_nybble_flag:
        .byte   $00

inverse_mask:
        .byte   $00

line_buffer_index:
        .byte   $12

; ----------------------------------------------------------------------------
; There's absolutely no reason why this data needs to be included in the
; binary load file: the line buffer's initial contents are meaningless, they
; will be blown away the first time anything reads from the E: device.

; Notice the author was running his debugger in COL80 when he built the
; binary (ASCII "S COL80 7A00 7F80" command still in the buffer).

 .ifdef FUJICHAT
line_buffer = $03FD ; cassette buffer
 .else
line_buffer:
        .byte   $53,$20,$43,$4F,$4C,$38,$30,$20
        .byte   $37,$41,$30,$30,$20,$37,$46,$38
        .byte   $30,$9B,$20,$20,$20,$20,$9B,$27
        .byte   $40,$40,$40,$40,$28,$28,$28,$28
        .byte   $40,$40,$40,$40,$40,$40,$40,$40
        .byte   $40,$40,$40,$40,$40,$40,$40,$40
        .byte   $9B,$FD,$FD,$FD,$FD,$9B
 .endif

END_ADDRESS = *-1

; I've found a variant (modified version?) of this code, that doesn't
; include the line_buffer in the file (no reason for it to be there),
; or the $0C segment, and that has another segment, loaded at $6000,
; with the run address changed to $6000.  The code looks like:

; .org $6000
; jsr dosini_entry_point
; lda #$50
; sta RMARGN
; lda #$00
; sta COLOR2

; also, the default colors have been changed in init_graphics_8.

; There are at least two binaries floating around that contain
; extra (garbage) bytes at the end, presumably from being transferred
; over XMODEM or similar. They are otherwise identical.

