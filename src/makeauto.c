#include <stdio.h>
#include <string.h>
#include <stdlib.h>

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

/* APPEND_BUF_SIZE should be >= the size of the largest .SER file */
#define APPEND_BUF_SIZE 2048

void reboot(void) {
	asm("jmp $e477");
}

static char get_yesno(char *prompt, char dflt) {
	char buf[5];

	printf("%s [%c/%c]: ",
			prompt,
			(dflt ? 'Y' : 'y'),
			(dflt ? 'n' : 'N'));

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

//char append_to(FILE *to, char *src) {
//	int bytes;
//	char ret;
//	static char buf[APPEND_BUF_SIZE];
//	FILE *from = fopen(src, "rb");
//
//	printf("append_to: %s\n", src);
//
//	if(!from) {
//		perror(src);
//		return 0;
//	}
//
//	while( (bytes = fread(buf, 1, APPEND_BUF_SIZE, from)) ) {
//		printf("%d bytes in, eof? %d\n", bytes, feof(from));
//		bytes = fwrite(buf, 1, bytes, to);
//		printf("%d bytes out\n", bytes);
//	}
//
//	ret = feof(from); /* 1 = no error */
//	fclose(from);
//
//	return ret;
//}

char append_to(FILE *to, char *src) {
	int bytes = 0, c;
	char ret;
	// static char buf[APPEND_BUF_SIZE];
	FILE *from = fopen(src, "rb");

	printf("append_to: %s\n", src);

	if(!from) {
		perror(src);
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

	printf("bytes=%d, ret=%d\n", bytes, ret);
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
		perror(dst);
		return 0;
	}
}

static void rs232_driver_menu() {
	static char buf[256];
	int i;

	puts("Select your serial port type\n");
	for(i=0; i<DRIVER_LIST_LEN; ++i) {
		printf("%d: %s\n",
				i + 1,
				rs232_drivers[i][1]);
	}

	puts("\nEnter number from list, or the\ndriver filename if not listed.");
	fputs("[2]: ", stdout);
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

	// if(!rename("D:AUTORUN.SYS", "D:AUTORUN.BAK")) {
		// puts("Renamed AUTORUN.SYS to AUTORUN.BAK");
	// }

	do {
		printf("\nCreating AUTORUN.SYS from\n%s and LOADMENU.COM\n", buf);
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

