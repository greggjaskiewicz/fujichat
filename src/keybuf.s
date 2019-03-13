	.fopt		compiler,"cc65 v 2.12.9"
	.setcpu		"6502"
	.smart		on
	.autoimport	on
	.case		on
	.debuginfo	off
	.importzp	sp, sreg, regsave, regbank, tmp1, ptr1, ptr2
	.export _keybuf_init, _keybuf_poll_kbd, _keybuf_cgetc

NO_KEY = $FF
max_buf_len = 12 ; aka buffer size

BUFFER_BASE = $F0

keybuf_head = BUFFER_BASE
keybuf_tail = BUFFER_BASE+1
keybuf = BUFFER_BASE+2
keybuf_end = keybuf + max_buf_len

CH = $02FC

	.segment "CODE"

; void __fastcall__ keybuf_init(void)
_keybuf_init:
	lda #keybuf
	sta keybuf_head
	sta keybuf_tail
	rts

; void __fastcall__ keybuf_poll_kbd(void)
_keybuf_poll_kbd:
	lda CH
	cmp #NO_KEY
	beq kbpoll_done

	; got a key, see if room in buffer
	ldx keybuf_tail
	cpx #keybuf_end+1
	bcs kbpoll_done

	; buffer not full, so store key in buffer
	sta 0,x
	inc keybuf_tail

	; clear keyboard register
	lda #NO_KEY
	sta CH

kbpoll_done:
	rts


; char __fastcall__ keybuf_is_empty(void)
_keybuf_is_empty:
	ldx #0
	sec
	lda keybuf_tail
	sbc keybuf_head
	rts


; char __fastcall__ keybuf_cgetc(void)

; Note: 0 means "no key pressed", which means we
; can't return character code 0 (not a big deal).
; If this is a problem, we could change the return
; type to int, and return -1 instead.

_keybuf_cgetc:
	ldx keybuf_head
	cpx keybuf_tail
	beq check_ch

	; got a key, stuff it in CH
	lda 0,x
	sta CH
	inc keybuf_head

call_cgetc:
	; ...and return the ASCII code
	jmp _fuji_cgetc

check_ch:
	jsr _keybuf_init

	ldx CH
	cpx #NO_KEY
	bne call_cgetc

	inx ; was $FF, now 0
	txa ; aka return(0)
	rts

