/*
 * Copyright (c) 2008, Brian Watson
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgements:
 *      This product includes software developed by Adam Dunkels.
 *      This product includes software developed by Brian Watson.
 * 4. The name of the author may not be used to endorse or promote
 *    products derived from this software without specific prior
 *    written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * This file is part of the FujiChat IRC client.
 *
 */


/* standard C includes */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* cc65 includes */
#include <rs232.h>
#include <peekpoke.h>
#include <atari.h>

/* uIP includes */
#include "timer.h"
#include "uip.h"
#include "uiplib.h"
#include "rs232dev.h"

/* FujiChat includes */
#include "fujichat.h"
#include "common.h"
#include "features.h"

#ifdef FEAT_KEYBOARD_MACROS
#include "keybuf.h"
#endif

#ifdef FEAT_COL80_HACK
/* $54 and $55 are ROWCRS and COLCRS.
	COL80 uses them the same way the OS does. */
# define fuji_gotox(x) POKE(0x55, x)
# define fuji_gotoy(y) POKE(0x54, y)
# define fuji_getxpos() PEEK(0x55)
# define fuji_getypos() PEEK(0x54)

/* TODO: calculate from RMARGN-LMARGN. Currently COL80 doesn't set RMARGN */
# ifndef COLUMNS
#  define COLUMNS 80
# endif
#endif


#ifdef FEAT_DYNAMIC_VERSION
char os_version[10] = "XL_XE";
#else
#define os_version "8-bit"
#endif

#ifdef FEAT_LOW_RAM_BUFFERS
#define LOW_RAM_BASE 0x2900
char *input_buf = (char *)LOW_RAM_BASE; // $2900 - $21FF
char *serv_msg_buf = (char *)(LOW_RAM_BASE + BUF_SIZE); // $2A00 - $2BFF
char *output_buf = (char *)(LOW_RAM_BASE + BUF_SIZE + BUF_SIZE * 2); // $2C00 - $2DFF
#else
char input_buf[BUF_SIZE];
char serv_msg_buf[BUF_SIZE * 2];
char output_buf[OUTBUF_SIZE];
#endif

char channel[HOSTLEN];

#ifdef FEAT_KEYBOARD_MACROS
char last_msg_nick[NICKLEN+1] = "";
#endif

int serv_msg_buf_len = 0;
int input_buf_len = 0;
int output_buf_len = 0;

char done = 0;
char connected = 0;
char joined_channel = 0; /* true if we're in a channel */
char nick_registered = 0;

#ifdef FEAT_VISUAL_BELL
char vbell_active = 0;
#endif

#ifdef FEAT_ATRACT_AWAY
char away = 0;
#endif

static void handle_keystroke(void);

#ifdef FEAT_LOGGING
static void alloc_log_buffer();
#endif

#if defined(FEAT_CURSOR_CONTROLS) || defined(FEAT_COL80_HACK)
void redraw_user_buffer();
void del_user_buffer();
#endif

/* uIP API stuff */
struct timer nick_reg_timer; /* ../uip/timer.h */
struct telnet_state *tstate; /* ../apps/telnet.h */

#ifdef FEAT_VISUAL_BELL
struct timer vbell_timer;
#endif

/*---------------------------------------------------------------------------*/
void main(void) {
#ifdef FEAT_DYNAMIC_VERSION
	char c;
#endif
	int i;
	uip_ipaddr_t ipaddr;
	struct timer periodic_timer;
	char cmdbuf[HOSTLEN+1];

	disable_break();

#ifdef FEAT_DYNAMIC_VERSION
	c = get_ostype() & AT_OS_TYPE_MAIN;
	if(c == AT_OS_400800) {
		strcpy(os_version, "800");
	} else if(c == AT_OS_1200XL) {
		strcpy(os_version, "1200XL");
	}
#endif

	/* set screen colors to white-on-black initially. They may be
		reset by the user's config file, once it's loaded */
	POKE(710, 0);
	POKE(709, 12);

#ifndef FEAT_COL80_HACK
	fuji_putchar(A_CLR);
#endif

	puts(BANNER);
	fuji_putchar('\n');

	if(!config_is_valid()) {
		if(!get_config())
			set_default_config();
	}

#ifdef FEAT_LOGGING
	alloc_log_buffer();
#endif

	// config->ui_flags |= UIFLAG_HIDEMOTD; // TODO: make it a preference!

	while(1) {
		done = 0;
		rs232dev_close();

		POKE(710, config->bg_color);
		POKE(709, config->fg_color);

		printf("Server name/IP, [C]onfig, or [D]OS\n");
		printf("[%s]: ", config->server);
		fflush(stdout);
		get_line(cmdbuf, HOSTLEN);

		if(strcasecmp(cmdbuf, "d") == 0) {
			exit(0);
		} else if(strcasecmp(cmdbuf, "c") == 0) {
			i = atari_exec(SETUP_FILENAME);
			printf("Error %d!\n", i);
			bell();
			continue;
		} else if(*cmdbuf) {
			strcpy(config->server, cmdbuf);
		}
		/* else, use the existing config->server */

		if(rs232dev_init(config->baud) != RS_ERR_OK)
			continue;

		timer_set(&periodic_timer, CLOCK_SECOND / 2);

		uip_init();

		memcpy(uip_hostaddr, &(config->local_ip), 4);
		memcpy(uip_draddr, &(config->peer_ip), 4);

		/* can I use 255.255.255.252 here? Does it matter? */
		uip_ipaddr(ipaddr, 255,255,255,0);
		uip_setnetmask(ipaddr);

		if(uiplib_ipaddrconv(config->server, (unsigned char*)&ipaddr)) {
			(void)uip_connect(&ipaddr, htons(config->server_port));
		} else {
			resolv_init();
			resolv_conf((u16_t *)&(config->resolver_ip)); // TODO: fix so I don't need extra ()
			resolv_query(config->server);
		}

		while(!done) {
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

#ifdef FEAT_VISUAL_BELL
			if(vbell_active && timer_expired(&vbell_timer)) {
				POKE(710, config->bg_color);
				vbell_active = 0;
			}
#endif

			/* Call the keyboard handler if the user pressed a key. Note that
				we're using stdio for printing, but conio for reading, which
				works on the Atari but may not work on all other cc65 platforms */
#ifdef FEAT_KEYBOARD_BUFFER
			/* always call; will return if nothing to do */
			handle_keystroke();
#else
			if(PEEK(764) != 0xff) { /* check CH (replaces kbhit()) */
				handle_keystroke();
			}
#endif

#ifdef FEAT_ATRACT_AWAY
			if(away && input_buf_len && !(PEEK(77) & 0x80)) {
				away = 0;
				send_server_cmd("AWAY", NULL);
			} else if(!away && (PEEK(77) & 0x80)) {
				away = 1;
				send_server_cmd("AWAY", "ATRACT Mode");
			}
#endif

			/* Automatically register the nick and userhost after being
				connected for a couple seconds. The parameters for the USER
				command are ignored by the server (at least the NewNet servers) */
			if(connected && !nick_registered && timer_expired(&nick_reg_timer)) {
				puts("> Registering nick");

				serv_msg_buf_len = sprintf(
						serv_msg_buf, "NICK %s%cUSER %s %s %s :%s%c",
						config->nick, NL, config->nick, SELF,
						config->server, config->real_name, NL);
				send_serv_msg_buf();

				/*
				sprintf(input_buf, "NICK %s%cUSER %s %s %s :%s%c",
						config->nick, NL, config->nick, SELF,
						config->server, config->real_name, NL);
				telnet_send(tstate, input_buf, strlen(input_buf));
				*/

				input_buf_len = 0;
				nick_registered = 1;
				/* TODO: check for failure registering the nick */
			}

			/*
			if(PEEK(53279) == 6) { // START pressed
				puts("Disconnecting");
				uip_close();
				// done = 1;
			}
			*/
		}

		connected = nick_registered = joined_channel = 0;
	}
	// return 0;
}

void bell() {
	if(config->ui_flags & UIFLAG_NOBELL)
		return;

#ifdef FEAT_VISUAL_BELL
	if(config->ui_flags & UIFLAG_VBELL) {
		vbell_active = 1;
		timer_set(&vbell_timer, CLOCK_SECOND / 10);
		POKE(710, config->bg_color + 8);
		return;
	}
#endif

#ifndef FEAT_COL80_HACK /* COL80 E: doesn't do the bell character */
	fuji_putchar(A_BEL);
#endif
}

static void err_no_channel(void) {
	puts("You are not in a channel (use /join #channel)");
	bell();
}

#ifdef FEAT_CURSOR_CONTROLS
static void del_last_word(void) {
	int old = input_buf_len, bs = 0;

	while(input_buf_len && input_buf[input_buf_len] == ' ') {
		++bs;
		input_buf_len--;
	}

	while(input_buf_len && input_buf[input_buf_len] != ' ') {
		++bs;
		input_buf_len--;
	}

	if(old > 120 && input_buf_len <= 120) {
		fuji_putchar(0x1c);
		fuji_putchar(A_DEL);
		fuji_putchar(A_DEL);
		redraw_user_buffer();
	} else do {
		fuji_putchar(A_BS);
	} while(--bs);
}
#endif

#ifdef FEAT_COL80_HACK
void col80_backspace(char destructive) {
	char x = fuji_getxpos();
	char y = fuji_getypos();

	if(x) {
		--x;
	} else {
		x = COLUMNS - 1;
		--y;
	}

	fuji_gotox(x);
	fuji_gotoy(y);
	if(destructive) {
		fuji_putchar(' ');
		fuji_gotox(x);
		fuji_gotoy(y);
	}
}

//void col80_cursor(void) {
//	fuji_putchar(0xa0); /* inverse space */
//	col80_backspace(0);
//}

#  define backspace() col80_backspace(1)

#else
#  define backspace() fuji_putchar(A_BS)
#endif

/* Keyboard handler. For now, the keyboard macros are hard-coded. */
static void handle_keystroke(void) {
	char i, c, send_buf = 0;

	/* TODO:
		ctrl-7 = ` (warning: cgetc() can't read this)
		ctrl-, = {
		ctrl-. = }
		ctrl-; = ~ (don't do ctrl-^, that's left-arrow!)

		Ignore inverse-video key, ATASCII graphics, unwanted
		cursor actions, make caps-lock stop freezing the program.
		To do this right, we have to examine location 764 for some
		of these, and don't call cgetc() at all... or actually,
		stuff a valid keycode there, then call cgetc() to make a
		keyclick noise (but ignore its return value).
		Addendum - this should be handled in keybuf.s?

		Stock col80 key mappings are warped:
		ctrl-clear = }
		ctrl-;     = {
		ctrl-.     = `
		No way to type a tilde because that's the A8 backspace char :(

	*/
#ifdef FEAT_KEYBOARD_BUFFER
	c = keybuf_cgetc();
	if(!c) return;
#else
	c = fuji_cgetc();
#endif

#ifdef FEAT_KEYBOARD_MACROS
	/* Keyboard macros, only allowed at the start of a new line */
	if(input_buf_len == 0) {
		switch(c) {
			case 0x0e: /* ctrl-N */
				if(joined_channel) send_server_cmd("NAMES", channel);
				return;

			case 0x17: /* ctrl-W */
				if(joined_channel) send_server_cmd("WHO", channel);
				return;

			case A_EOL:
				return; /* ignore Return on a line by itself */

			case A_TAB:
				if(last_msg_nick[0]) {
					sprintf(input_buf, "/msg %s ", last_msg_nick);
					input_buf_len = strlen(input_buf);
					for(i=0; i<input_buf_len; i++)
						fuji_putchar(input_buf[i] | 0x80);
				}
				return;

			default:
				break;
		}
	}
#else
	if(!input_buf_len && c == A_EOL)
		return;
#endif

	/* Editing keys */
	/* TODO: more editing keys. At minimum:
		left/right arrows
		^W = delete last word
		^U = kill to start of line
		^K = kill to end of line
		^A = goto start of line
		^E = goto end of line
		^L = redraw input buffer
	 */
	if(c == A_BS) { /* backspace */
		if(input_buf_len > 0) {
			input_buf_len--;
#ifdef FEAT_COL80_HACK
			col80_backspace(1);
			// fuji_putchar(' ');
			// col80_backspace(0);
			// col80_cursor();
#else
			fuji_putchar(c);
#endif
		} else {
			bell();
		}

		return;
	} else if(c == A_DEL || c == 0x15) {
		/* shift-backspace or ^U (delete line) */
#ifdef FEAT_COL80_HACK
		del_user_buffer();
		// col80_cursor();
#else
		fuji_putchar(A_DEL);
#endif
		input_buf_len = 0;
		return;
#ifdef FEAT_CURSOR_CONTROLS
	} else if(c == 0x17) {
		/* ^W (delete word) */
		del_last_word();
		return;
#endif
	}

	/* Echo keystroke */
	fuji_putchar(c | 0x80); /* inverse video for now */

	if(c == A_EOL) {
		c = NL;
		send_buf = 1;
	} else if(c == A_TAB) {
		c = TAB;
	}
// #ifdef FEAT_COL80_HACK
	// else { col80_cursor(); }
// #endif


#ifdef FEAT_UNICODE_TEST
	if(c == 0x10) { /* ATASCII clubs symbol */
		input_buf[input_buf_len++] = 0xe2; /* UTF-8 marker */
		input_buf[input_buf_len++] = 0x99;
		input_buf[input_buf_len++] = 0xa3;
		fuji_putchar(c);
		return;
	}
#endif

	/* Store keystroke in input buffer */
	input_buf[input_buf_len++] = c;

	/* If line too long, ring the "margin" bell */
	if(input_buf_len > BUF_SIZE - 1)
		if(!send_buf) {
			bell();
			// fuji_putchar(A_BS);
			backspace();
			input_buf_len--;
		}

	/* If we've got a complete line of input and user has pressed Return,
		send it to the server */
	if(send_buf) {
		input_buf[input_buf_len] = '\0';

		if(*input_buf == '/') {
			handle_command();
		} else if(joined_channel) {
			sprintf(serv_msg_buf, "PRIVMSG %s :%s", channel, input_buf);
			telnet_send(tstate, serv_msg_buf, strlen(serv_msg_buf));
		} else {
			err_no_channel();
		}

		input_buf_len = 0;
	}
}

void send_serv_msg_buf(void) {
	telnet_send(tstate, serv_msg_buf, serv_msg_buf_len);
}

void send_server_cmd(char *cmd, char *arg) {
	if(arg) {
		serv_msg_buf_len = sprintf(serv_msg_buf, "%s %s%c", cmd, arg, NL);
	} else {
		serv_msg_buf_len = sprintf(serv_msg_buf, "%s%c", cmd, NL);
	}

	send_serv_msg_buf();
}

/* The telnet_* functions are uIP application callbacks. */
void telnet_connected(struct telnet_state *s) {
	/* puts("Connected to host, press START to disconnect"); */
	puts("> Connected to server");
	tstate = s;
	s->text = NULL;
	s->textlen = 0;
	timer_set(&nick_reg_timer, CLOCK_SECOND);
	connected = 1;
}

/* 20081125 bkw: why don't these pragmas do anything? */
#pragma warn (off)
void telnet_closed(struct telnet_state *s) {
	puts("> Connection closed");
	uip_close();
	done = 1;
}

void telnet_sent(struct telnet_state *s) {
}

void telnet_aborted(struct telnet_state *s) {
	puts("> Connection aborted");
	uip_abort();
	done = 1;
}

void telnet_timedout(struct telnet_state *s) {
	puts("> Connection timed out");
	uip_abort();
	done = 1;
}
#pragma warn (on)

void do_pong() {
	char *p = serv_msg_buf;

	memcpy(serv_msg_buf, output_buf, output_buf_len);
	serv_msg_buf[1] = 'O';
	while(*p) {
		if(*p == A_EOL) *p = NL;
		p++;
	}

	telnet_send(tstate, serv_msg_buf, output_buf_len);
	if(config->ui_flags & UIFLAG_SHOWPING)
		puts("[PING,PONG]");
}

void do_server_msg() {
	char *cmd = output_buf, *arg;
	int numeric = 0, len;

	while(*cmd != ' ')
		++cmd;

	++cmd;

	arg = cmd;
	while(*arg != ' ')
		++arg;

	*arg = '\0';
	++arg;

	len = strlen(config->nick);
	if(memcmp(arg, config->nick, len) == 0)
		arg += (len + 1); // skip nick if present

	if(arg[0] == ':')
		++arg;

	/* now cmd points to the command (numeric or whatever),
		arg points to the rest of the string.
		Numeric commands we handle now I guess. I think they
		all have the nick as the 1st argument...
	 */
	if( (numeric = atoi(cmd)) ) {
		switch(numeric) {
			case 332: // topic
				printf("> Topic: %s", arg);
				return;
				break;

			case 333: // topic nick
				printf("> Topic set by %s", arg);
				return;
				break;

			case 375: // start of MOTD
			case 372: // MOTD text
				if(config->ui_flags & UIFLAG_HIDEMOTD) {
					if(numeric == 375)
						puts("> Hiding MOTD (be patient)");
				}
				return;
				break;

			case 376: // end of MOTD
			case 422: // or MOTD missing
				if(*(config->channel)) {
					strcpy(channel, config->channel);
					printf("> Joining %s\n", channel);
					send_server_cmd("JOIN", channel);
					joined_channel = 1;
				}
				break;

				/*
			case 352: // /who list
			case 353: // /names list
			case 366: // end of /names
			case 315: // end of /who
				break;
				*/

			default:
				break;
		}
	}

	printf("%s %s", cmd, arg);
}

char incoming_ctcp(char *nick, char *msg) {
	long now, then;
	int sec, dec;

	if(memcmp(msg, "PING ", 5) == 0) {
		now = clock() * 10L;
		then = ((long)(atoi(msg + 5)) +
			    ((long)(atoi(msg + 9)) << 8) +
			    ((long)(atoi(msg + 13)) << 16)) * 10L;

		now -= then;
		now /= CLOCKS_PER_SEC;
		sec = now / 10;
		dec = now % 10;

		/* silently ignore timestamp wraparound */
		/*
		if(then >= now)
			return 1;
			*/

		printf("> %s ping time: %d.%d\n", nick, sec, dec);
		return 1;
	}

	return 0;
}

/* Handler for text received from the server, responsible for
	formatting. TODO: part/join/quit aren't handled correctly. */
void do_msg() {
	char *nick = output_buf, *cmd = output_buf, *chan = NULL, *msg = NULL;
	char *bang = NULL;

	while(*cmd != ' ') {
		if(*cmd == '!') {
			bang = cmd;
		}
		++cmd;
	}

	/* no ! in nick means a server message */
	if(!bang) {
		do_server_msg();
		return;
	}

	*cmd = '\0';
	++cmd;
	chan = cmd;

	while(*chan != ' ')
		++chan;
	*chan = '\0';
	++chan;
	msg = chan;

	if(*msg != A_EOL) {
		while(*msg && *msg != ' ')
			++msg;
		if(*msg) {
			*msg = '\0';
			++msg;
			if(*msg == ':') ++msg;
		}
	}

	/* FIXME: This stuff is hairy, unmaintainable, and probably just wrong
		(works OK with the 2 or 3 NewNet servers I test with, not tried
		others). It's the result of too much coffee, and needs to be
		redesigned. Heck, it needs to be designed in the first place! */
	if(strcmp(cmd, "PRIVMSG") == 0) {
		nick[0] = '<';
		bang[0] = '>';
		bang[1] = '\0';
		if(strcasecmp(chan, config->nick) != 0) {
			// privmsg, not to our nick, must be channel text
			if(memcmp(msg, "\x01" "ACTION ", 8) == 0) {
				nick++;
				*bang = '\0';
				output_buf[output_buf_len - 2] = A_EOL;
				output_buf[output_buf_len - 1] = '\0';
				printf("* %s %s", nick, msg + 8);
			} else {
				printf("%s %s", nick, msg);
			}
		} else {
			// privmsg, is to our nick
			if(memcmp(msg, "\x01" "PING ", 6) == 0) {
				nick++;
				bang[0] = '\0';
				output_buf[output_buf_len - 2] = '\0';
				serv_msg_buf_len = sprintf(serv_msg_buf, "NOTICE %s :\x01PING %s\x01%c",
						nick, msg + 6, 0x0a);
				send_serv_msg_buf();
				printf("* CTCP PING from %s\n", nick);
			} else if(memcmp(msg, "\x01" "VERSION\x01", 9) == 0) {
				nick++;
				*bang = '\0';
				serv_msg_buf_len = sprintf(serv_msg_buf, "NOTICE %s :\x01VERSION "
						VERSION_REPLY " - running on "
						"an Atari %s\x01%c",
						nick, os_version, 0x0a);
				send_serv_msg_buf();
				printf("* CTCP VERSION from %s\n", nick);
			} else {
				printf("-> %s %s", nick, msg);
				if(config->ui_flags & UIFLAG_MSGBELL)
					bell();
				bang[0] = '\0';
#ifdef FEAT_KEYBOARD_MACROS
				strcpy(last_msg_nick, nick + 1);
#endif
			}
		}
	} else if((strcmp(cmd, "NOTICE") == 0) && msg[0] == '\x01') {
		nick++;
		*bang = '\0';
		if(!incoming_ctcp(nick, msg+1))
			printf("* CTCP reply from %s: %s", nick, msg+1); // still has A_EOL
	} else {
		if(*msg) {
			printf("%s %s %s %s", nick, cmd, chan, msg);
		} else {
			printf("%s %s %s", nick, cmd, chan);
		}
	}

	fflush(stdout);
}

#ifdef FEAT_CURSOR_CONTROLS
void del_user_buffer() {
	/* delete user edit buffer */
	if(input_buf_len) {
		fuji_putchar(A_DEL);
		if(input_buf_len > 119) {
			fuji_putchar(0x1c); /* up arrow */
			fuji_putchar(A_DEL);
		}
	}
}

#elif defined(FEAT_COL80_HACK)

/* COL80 doesn't support the delete-line or arrow key
	control codes.

	Deleting and redrawing the user buffer is done by:

	1. Move the cursor to the start of the buffer on screen
	2. Print input_buf_len spaces
	3. Move cursor to the same spot as (1)
	4. Print the incoming text including EOL as usual
	5. redraw_user_buffer() as usual
*/

void del_user_buffer() {
	int tlen;
	char oldy;

	if(!input_buf_len)
		return;

	tlen = input_buf_len;
	oldy = fuji_getypos() - (input_buf_len / COLUMNS);

	fuji_gotox(0);
	fuji_gotoy(oldy);

	do {
		fuji_putchar(' ');
	} while(--tlen);

	fuji_gotox(0);
	fuji_gotoy(oldy);
}

#endif

#if defined(FEAT_CURSOR_CONTROLS) || defined(FEAT_COL80_HACK)
//void redraw_user_buffer() {
//	int tlen;
//#  ifdef FEAT_LOW_RAM_BUFFERS
//#    define redraw_buf ((char *) 0x2800)
//#  else
//	static char redraw_buf[BUF_SIZE];
//#  endif
//	/* reprint user edit buffer */
//	if(input_buf_len) {
//		tlen = input_buf_len - 1;
//		do {
//			redraw_buf[tlen] = input_buf[tlen] | 0x80;
//		} while(tlen--);
//		redraw_buf[input_buf_len] = '\0';
//		fputs(redraw_buf, stdout);
//		fflush(stdout);
//	}
//}

void redraw_user_buffer() {
	int i;
	if(input_buf_len) {
		for(i=0; i<input_buf_len; ++i)
			fuji_putchar(input_buf[i] | 0x80);
	}
}

#endif

/* Another uIP app callback. This decides whether or not we've received
	a complete message from the server (which may be split into multiple
	packets, and may end in the middle of a packet) */
void telnet_newdata(struct telnet_state *s, char *data, u16_t len) {
	u16_t tlen = len;
	char c, *t = data, buf_done = 0;

	while(tlen-- != 0) {
		c = *t++;
		if(c == NL) { // swallow NL
			continue;
		} else if(c == CR) { // CR => EOL
			c = A_EOL;
			buf_done = 1;
		} else if(c == TAB) { // tab
			c = A_TAB;
#ifndef FEAT_COL80_HACK
			/* COL80 can actually print these OK */
		} else if(c == '{') {
			c = '[' | 0x80;
		} else if(c == '}') {
			c = ']' | 0x80;
		} else if(c == '~') {
			c = '^' | 0x80;
		} else if(c == 0x60) { // backtick
			c = 0xa7; // inverse quote
#endif
		}

		output_buf[output_buf_len++] = c;
		if(output_buf_len >= OUTBUF_SIZE) {
			puts("[buffer overflow]\xfd");
			buf_done = 1;
			break;
		}

		if(buf_done) {
#if defined(FEAT_CURSOR_CONTROLS) || defined(FEAT_COL80_HACK)
			del_user_buffer();
#endif
			output_buf[output_buf_len] = '\0';

			if(output_buf_len > 4 && (memcmp(output_buf, "PING", 4) == 0)) {
				do_pong();
			} else if(output_buf[0] == ':') {
				do_msg();
			} else {
				fputs(output_buf, stdout);
				fflush(stdout);
			}

#if defined(FEAT_CURSOR_CONTROLS) || defined(FEAT_COL80_HACK)
			redraw_user_buffer();
#endif

#ifdef FEAT_COL80_HACK
			// col80_cursor();
#endif
			output_buf_len = 0;
			buf_done = 0;
		}
	}
}


/*---------------------------------------------------------------------------*/

/* uIP app callback for the resolver */
void resolv_found(char *name, u16_t *ipaddr) {
	if(ipaddr == NULL) {
		printf("Host '%s' not found.\n", name);
		done = 1;
	} else {
		printf("%s is %s\n", name, format_ip((uip_ipaddr_t *)ipaddr));
		/*
		printf("Found name '%s' = %d.%d.%d.%d\n", name,
				htons(ipaddr[0]) >> 8,
				htons(ipaddr[0]) & 0xff,
				htons(ipaddr[1]) >> 8,
				htons(ipaddr[1]) & 0xff);
				*/
		if(!connected)
			(void)uip_connect((uip_ipaddr_t *)ipaddr, htons(config->server_port));
	}
}

#ifdef FEAT_LOGGING

/* 0x100 * 32 = 8K (but we likely won't get that much) */
#define LOG_PAGE_SIZE 0x200
#define LOG_MAX_PAGES 16
#define LOG_SCROLL_PAGES 2

static char *log_buffer = NULL;
static char *log_buf_end = NULL;
static char *log_buf_current = NULL;
static int log_pages = LOG_MAX_PAGES;

static char last_log_nick[NICKLEN + 1];

static void alloc_log_buffer() {
	do {
		printf("Trying to malloc(%x)...", LOG_PAGE_SIZE * log_pages);

		log_buffer = malloc(LOG_PAGE_SIZE * log_pages);

		if(log_buffer) {
			puts("OK");
		} else {
			puts("Failed");
		}
	} while(!log_buffer && (--log_pages > LOG_SCROLL_PAGES * 2));

	if(!log_buffer) return;

	log_buf_end = (char *)(log_pages * LOG_PAGE_SIZE - 1);
	log_buf_current = log_buffer;

	printf("log_buffer == %x (%x)", log_buffer, log_pages * LOG_PAGE_SIZE);
	fuji_cgetc();
}

static void log_msg(char *msg) {
	int len = strlen(msg);
	if( (log_buf_current + len + 1) > log_buf_end ) {
		memmove(log_buffer, log_buffer + (LOG_SCROLL_PAGES * LOG_PAGE_SIZE), (log_pages - LOG_SCROLL_PAGES) * LOG_PAGE_SIZE);
		log_buf_current -= (LOG_SCROLL_PAGES * LOG_PAGE_SIZE);
	}
}

#endif
