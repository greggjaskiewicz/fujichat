/* Compile with:
	gcc -o clear_rts clear_rts.c

	Change PORT if necessary.
 */
#define PORT "/dev/ttyUSB0"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <linux/serial.h>

int main(int argc, char **argv) {
	int fd, status;
	char *port;

	if(argc == 2)
		port = argv[1];
	else
		port = PORT;

	fd = open(PORT, O_RDWR);
	if(fd < 0) {
		perror(PORT);
		return 1;
	}

	ioctl(fd, TIOCMGET, &status);

	/* This always says it was on. Opening the port sets it? */
	/*
	fprintf(stderr, "%s: RTS was ", port);
	if(status & TIOCM_RTS) {
		fputs("on", stderr);
	} else {
		fputs("already off", stderr);
	}
	fputc('\n', stderr);
	*/

	status &= ~TIOCM_RTS;
	ioctl(fd, TIOCMSET, &status);
	fprintf(stderr, "%s: RTS forced off (Tucker SIO2PC)\n", port);
	return 0;
}
