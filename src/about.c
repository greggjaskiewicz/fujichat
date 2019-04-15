#include <stdio.h>
#include <conio.h>
#include <string.h>
#include "fujichat.h"
#include "common.h"

/* TODO: rewrite in asm. This does NOT need to
	be 5K of object code. */

void more(char *file) {
	char c, line = 0;
	char buf[200];
	FILE *f = fopen(file, "r");

	putchar(0x7d);

	if(!f) {
		fputs("Error reading ", stdout);
		puts(file);
		cgetc();
		return;
	}

	while(fgets(buf, 200, f)) {
		if(*buf == '#') continue;

		line += (strlen(buf) / 40 + 1);
		if(line > 22) {
			fputs("--more--", stdout);
			fflush(stdout);

			c = cgetc();
			if(c == 27)
				break;

			putchar(A_DEL);
			line = 0;
		}

		fputs(buf, stdout);
	}

	fclose(f);

	fputs("--any key--", stdout);
	fflush(stdout);

	c = cgetc();
}

int main(int, char **) {
	more("D:ABOUT.TXT");
	atari_exec(MENU_FILENAME);
	return 0;
}
