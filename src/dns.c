#include <stdio.h>
#include <stdlib.h>
#include <atari.h>
#include "uip.h"
#include "uiplib.h"
#include "rs232dev.h"
#include <conio.h>
#include <peekpoke.h>
#include <string.h>
#include <ctype.h>

#include "telnet.h"
#include "resolv.h"
#include "timer.h"

#ifndef NULL
#define NULL (void *)0
#endif /* NULL */

char buf[256];

int main(void) {
	char c;
	int i;
	uip_ipaddr_t ipaddr;
	struct timer periodic_timer;

	timer_set(&periodic_timer, CLOCK_SECOND / 2);

	rs232dev_init();
	uip_init();

	uip_ipaddr(ipaddr, 192,168,0,2);
	uip_sethostaddr(ipaddr);
	uip_ipaddr(ipaddr, 192,168,0,1);
	uip_setdraddr(ipaddr);
	uip_ipaddr(ipaddr, 255,255,255,0);
	uip_setnetmask(ipaddr);

	resolv_init();
	uip_ipaddr(ipaddr, 192,168,88,1);
	resolv_conf(ipaddr);

	cursor(1);

	while(1) {
		puts("DNS test. Hostname:");
		fgets(buf, 256, stdin);
		for(i=0; i<strlen(buf); ++i)
			if(buf[i] == '\n') buf[i] = '\0';
		printf("Trying to resolve host %s, press any key to stop\n", buf);
		resolv_query(buf);

		while(1) {
			/* This part of the while loop is straight from the no-ARP example
				code, from the uIP docs. */
			uip_len = rs232dev_poll();
			if(uip_len > 0) {
				uip_input();
				/* If the above function invocation resulted in data that
					should be sent out on the network, the global variable
					uip_len is set to a value > 0. */
				if(uip_len > 0) {
					rs232dev_send();
				}
			} else if(timer_expired(&periodic_timer)) {
				timer_reset(&periodic_timer);
				for(i = 0; i < UIP_CONNS; i++) {
					uip_periodic(i);
					/* If the above function invocation resulted in data that
						should be sent out on the network, the global variable
						uip_len is set to a value > 0. */
					if(uip_len > 0) {
						rs232dev_send();
					}
				}

#if UIP_UDP
				for(i = 0; i < UIP_UDP_CONNS; i++) {
					uip_udp_periodic(i);
					/* If the above function invocation resulted in data that
						should be sent out on the network, the global variable
						uip_len is set to a value > 0. */
					if(uip_len > 0) {
						rs232dev_send();
					}
				}
#endif /* UIP_UDP */
			}

			if(kbhit()) {
				cgetc();
				break;
			}
		}
	}
}

void telnet_connected(struct telnet_state *s) {
}

void telnet_closed(struct telnet_state *s) {
}

void telnet_sent(struct telnet_state *s) {
}

void telnet_aborted(struct telnet_state *s) {
}

void telnet_timedout(struct telnet_state *s) {
}

void telnet_newdata(struct telnet_state *s, char *data, u16_t len) {
}

void resolv_found(char *name, u16_t *ipaddr) {
	if(ipaddr == NULL) {
		printf("Host '%s' not found.\n", name);
	} else {
		printf("Found name '%s' = %d.%d.%d.%d\n", name,
				htons(ipaddr[0]) >> 8,
				htons(ipaddr[0]) & 0xff,
				htons(ipaddr[1]) >> 8,
				htons(ipaddr[1]) & 0xff);
		puts("Press any key...");
		// webclient_get("www.google.com", 80, "/index.html");
		// webclient_get(name, 80, "/~adam/uip");
	}
}
