	.fopt		compiler,"cc65 v 2.12.9"
	.setcpu		"6502"
	.smart		on
	.autoimport	on
	.case		on
	.debuginfo	off
	.importzp	sp, sreg, regsave, regbank, tmp1, ptr1, ptr2
	.macpack	longbranch
	.export		_fuji_putchar

.segment	"CODE"
_fuji_putchar:
	tax
	;lda $E407 ; don't use ROM, might be running COL80
	lda $0347  ; grab from IOCB 0 instead
	pha
	;lda $E406
	lda $0346
	pha
	txa
	rts
