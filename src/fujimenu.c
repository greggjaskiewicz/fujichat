#include <stdio.h>
#include <conio.h>
#include <string.h>
#include <stdlib.h>
#include "fujichat.h"
#include "common.h"

/* TODO:
	- rewrite in asm
*/

void run_prog(char *file) {
	static char err[4];
	int c;
	fputs("\nLoading ", stdout);
	fputs(file, stdout);
	puts("...");
	c = atari_exec(file);
	// printf("Error %d!\n\xfd", c);
	(void)itoa(c, err, 10);
	fputs("Error ", stdout);
	fputs(err, stdout);
	putchar('!');
	putchar(0xfd);
	putchar('\n');
}

void main(void) {
	int c, have_conf;

	disable_break();

	puts("\x7d" BANNER " Main Menu\n");

	have_conf = get_config();
	if(!have_conf)
		set_default_config();

	while(1) {
		putchar('\n');
		putchar(' ');
		putchar(' ');
		putchar(0xc1);
		puts("bout");
		// printf("\n  %cbout\n", 'A' | 0x80);

		putchar(have_conf ? ' ' : '*');
		putchar(' ');
		putchar(0xd3);
		puts("etup");
		/*
		printf("%s%cetup\n",
				(have_conf ? "  " : "* "),
				'S' | 0x80);
				*/

		putchar(have_conf ? '*' : ' ');
		putchar(' ');
		putchar(0xc3);
		puts("hat");
		/*
		printf("%s%chat\n",
				(have_conf ? "* " : "  "),
				'C' | 0x80);
				*/

		puts("  \xc4OS");
		// printf("  %cOS\n", 'D' | 0x80);

		putchar('\n');
		putchar('>');

		cursor(1);
		c = cgetc();
		if(c == A_EOL || c == ' ')
			c = (have_conf ? 'C' : 'S');

		putchar(c | 0x80);

		switch(c) {
			case 'A':
			case 'a':
				run_prog("D:ABOUT.COM");
				break;

			case 'S':
			case 's':
				run_prog(SETUP_FILENAME);
				break;

			case 'C':
			case 'c':
				run_prog(IRC_FILENAME);
				// run_prog(IRC_LOADER_FILENAME);
				break;

				/*
				if(config_is_valid()) {
					// atari_exec(IRC_LOADER_FILENAME);
					atari_exec(IRC_FILENAME);
					return 0;
				} else {
					putchar(A_BS);
					puts("\nYou must run Setup first");
					break;
				}
				 */

			case 'D':
			case 'd':
				return;

			default:
				putchar(A_BS);
				break;
		}
	}
}
