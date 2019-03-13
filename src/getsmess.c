#include <stdio.h>
#include <peekpoke.h>


void getlnasm(char *buf, char len) {
	asm("ldy #$00");
	asm("lda (sp),y");
	asm("sta $e2");

	asm("ldy #$02");
	asm("jsr ldaxysp");
	asm("sta $e0");
	asm("stx $e0+1");

	asm("@loop: lda _stdin");
	asm("ldx _stdin+1");
	asm("jsr _fgetc");

	asm("cmp #$9B");
	asm("beq @out");

	asm("ldy #$00");
	asm("sta ($e0),y");
	asm("inc $e0");
	asm("bne @noinc");
	asm("inc $e0+1");
	asm("@noinc:");

	asm("dec $e2");
	asm("bne @loop");

	asm("@out:");
	asm("ldy #$00");
	asm("tya");
	asm("sta ($e0),y");
}

void getln(char *buf, char len) {
	char *p = buf;

	do {
		*p = fgetc(stdin);
		if(*p == '\n') break;
	} while(++p, --len);
	*p = '\0';
}

void main(void) {
	char buf[100] = "HELLO";
	while(1) {
		/*
		puts("gets");
		gets(buf);
		puts(buf);
		puts("fgets");
		fgets(buf, 99, stdin);
		puts(buf);
		puts("getln");
		getln(buf, 99);
		puts(buf);
		*/
		puts("getlnasm");
		getlnasm(buf, 99);
		puts(buf);
	}
}
