	.fopt		compiler,"cc65 v 2.12.9"
	.setcpu		"6502"
	.smart		on
	.autoimport	on
	.case		on
	.debuginfo	off
	.importzp	sp, sreg, regsave, regbank, tmp1, ptr1, ptr2
	.macpack	longbranch
	.export		_new_format_ip

	.segment "BSS"
buf:
	.res 20,$0

	.segment "RODATA"
fmt: .byte "%d.%d.%d.%d", 0

; unsigned char* __fastcall__ format_ip (__near__ unsigned int[2]*)
	.segment "CODE"
_new_format_ip:
	sta ptr1
	stx ptr1+1

	lda #<buf
	ldx #>buf
	jsr pushax

	lda #<fmt
	ldx #>fmt
	jsr pushax

	ldy #0
	lda (ptr1),y
	jsr pusha0

	ldy #1
	lda (ptr1),y
	jsr pusha0

	ldy #2
	lda (ptr1),y
	jsr pusha0

	ldy #3
	lda (ptr1),y
	jsr pusha0

	ldy #$0c
	jsr _sprintf

	lda #<buf
	ldx #>buf

	rts
