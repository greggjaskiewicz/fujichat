#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include "fujichat.h"
#include "common.h"
#include "features.h"

#define DRIVER_LIST_LEN (sizeof(rs232_drivers) / sizeof(rs232_drivers[0]))
static char *rs232_drivers[][2] = {
	{ NULL, "(No driver; manual setup)" },
	{ "D:BOBVERT.SER", "SIO2PC or R:Verter" },
	{ "D:ATARI850.SER", "Atari 850" },
	{ "D:PRCONN.SER", "P:R: Connection" },
};

void reboot(void) {
	asm("jmp $e477");
}

static char get_yesno(char *prompt, char dflt) {
	char buf[5];

	fputs(prompt, stdout);
	fputs(" [", stdout);
	putchar(dflt ? 'Y' : 'y');
	putchar('/');
	putchar(dflt ? 'n' : 'N');
	fputs("]: ", stdout);

	fflush(stdout);
	get_line(buf, 4);
	switch(*buf) {
		case 'y':
		case 'Y':
			return 1;

		case 'n':
		case 'N':
			return 0;

		default:
			return dflt;
	}
}

void printerr(char *msg) {
	char ebuf[4];
	(void)itoa(_oserror, ebuf, 10);
	fputs(msg, stdout);
	fputs(": Error ", stdout);
	puts(ebuf);
	putchar(A_BEL);
}

char append_to(FILE *to, char *src) {
	int bytes = 0, c;
	char ret;
	FILE *from = fopen(src, "rb");

	if(!from) {
		printerr(src);
		return 0;
	}

	/* painfully slow, sorry! TODO: figure out why fread() returns
		extra bytes after EOF (probably because the file doesn't
		end on a sector boundary, or something equally lame) */
	while( (c = fgetc(from)) != EOF ) {
		fputc(c, to);
		++bytes;
	}

	ret = feof(from); /* 1 = no error */
	fclose(from);

	return ret;
}

char append_files(char *dst, char *src1, char *src2) {
	char ret;
	FILE *dfile = fopen(dst, "wb");

	if(dfile) {
		ret = (append_to(dfile, src1) && append_to(dfile, src2));
		fclose(dfile);
		return ret;
	} else {
		printerr(dst);
		fclose(dfile);
		return 0;
	}
}

static void rs232_driver_menu() {
	static char buf[256];
	int i;

	puts("Select your serial port driver");
	for(i=0; i<DRIVER_LIST_LEN; ++i) {
		putchar('1' + i);
		putchar(':');
		putchar(' ');
		puts(rs232_drivers[i][1]);
	}

	fputs("\nEnter number from list, or the\ndriver filename if not listed.\n[2]: ", stdout);
	fflush(stdout);

	do {
		i = 0;
		get_line(buf, 255);

		if(!*buf) *buf = '2';

		if(*buf == '1') {
			return;
		} else if(*buf >= '2' && *buf <= '9') {
			i = *buf - '1';
			if(i < DRIVER_LIST_LEN) {
				strcpy(buf, rs232_drivers[i][0]);
				i = 0;
			} else {
				putchar(A_BEL);
				i = 1;
			}
		} /* else use whatever they entered as a filename */
	} while(i);

	do {
		puts("\nCreating AUTORUN.SYS from");
		fputs(buf, stdout);
		puts(" and LOADMENU.COM");

		i = !append_files("D:AUTORUN.SYS", buf, "D:LOADMENU.COM");
		if(i) {
			i = get_yesno("\xfdRetry", 1);
			if(!i) {
				puts("Failed to change driver");
				return;
			}
		}
	} while(i);

	puts("You must reboot to use the new driver.");
	if(get_yesno("Reboot now", 1))
		reboot();

	// rs232_already_loaded = scan_hatabs('R');
}

void main(int, char **) {
	rs232_driver_menu();
	exit(0);
}

