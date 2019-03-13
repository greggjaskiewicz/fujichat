/* Compile with:
	gcc -o toggle_rts toggle_rts.c

	Change PORT if necessary.
 */
#define PORT "/dev/ttyS0"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <linux/serial.h>

int main() {
	int fd, status;

	fd = open(PORT, O_RDWR);
	if(fd < 0) {
		perror(PORT);
		return 1;
	}

	// fprintf(stderr, "ioctl(fd, TIOCMGET, &status);\n");
	ioctl(fd, TIOCMGET, &status);
	/*
	if(status & TIOCM_RTS) {
		fprintf(stderr, "RTS set\n");
	} else {
		fprintf(stderr, "RTS clear\n");
	}
	*/
	status &= ~TIOCM_RTS;
	ioctl(fd, TIOCMSET, &status);
	// fprintf(stderr, "ioctl(fd, TIOCMSET, &status);\n");
	fprintf(stderr, "RTS forced off (Tucker SIO2PC)\n");
	return 0;
}
