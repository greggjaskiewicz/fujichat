
#include <stdio.h>

extern unsigned char * __fastcall__ new_format_ip (void *);

void main(void) {
	char ip[4] = { 0xc0, 0xa8, 0x00, 0x01 };
	puts(new_format_ip(ip));
HANG: goto HANG;
}
