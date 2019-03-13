#include <stdio.h>
#include <peekpoke.h>
#include <conio.h>

#include "keybuf.h"

#define RTCLOK 0x14

void read_input(void) {
	char got[20];
	char c, *p = got;

	while( (c = keybuf_cgetc()) )
		putchar(c);

		// *p++ = c;

	/*
	*p = '\0';

	if(p != got)
		fputs(got, stdout);
		*/
}

void process(void) {
	POKE(RTCLOK, 0);
	while(PEEK(RTCLOK) < 30) {
		keybuf_poll_kbd();
	}
}

void idle(void) {
	char c;
	POKE(RTCLOK, 0);
	while(PEEK(RTCLOK) < 30) {
		if( (c = keybuf_cgetc()) )
			putchar(c);
	}
}

void main(void) {
	cursor(1);
	keybuf_init();

	while(1) {
		process();
		read_input();
		idle();
	}
}
