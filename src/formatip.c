#include <stdio.h>
#include "uip.h"

/* easier to copy/paste this tiny function from uip.c
	than it would be to rebuild all of uIP for use in
	this program! Also don't want to bloat fujiconf by
	linking uip.a, even if it would link without a
	recompile. */
/*
u16_t local_htons(u16_t val) {
	return HTONS(val);
}
*/

/* this version's half the size */
static u16_t local_htons(u16_t val) {
	__AX__ = val;
	asm("sta tmp1");
	asm("txa");
	asm("ldx tmp1");
}

static char ipbuf[20];

char *format_ip(uip_ipaddr_t *ip) {
	u16_t *ipaddr = (u16_t *)ip;
	sprintf(ipbuf, "%d.%d.%d.%d",
			local_htons(ipaddr[0]) >> 8,
			local_htons(ipaddr[0]) & 0xff,
			local_htons(ipaddr[1]) >> 8,
			local_htons(ipaddr[1]) & 0xff);

	return ipbuf;
}
