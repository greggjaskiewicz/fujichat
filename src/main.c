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
#include <rs232.h>

#include "timer.h"

#ifndef NULL
#define NULL (void *)0
#endif /* NULL */

/* Program name (plain ASCII for network use) */
#define SELF "FujiChat"
/* inverse video version, used for local prompts: */
#define SELF_INV "\xc6\xf5\xea\xe9\xc3\xe8\xe1\xf4"

#define VERSION "0.3.1"
#define BANNER SELF_INV " v" VERSION
#define VERSION_REPLY SELF " v" VERSION


#define DEFAULT_NICK SELF

#define CONF_SIGNATURE "\x03\x0e"
#define CONF_VERSION 4
#define DEFAULT_CONF_FILE "FUJICHAT.CFG"

/* TODO: move the non-IRC-related config to a separate SETUP program,
	and a separate UIP.CFG file. This will be shared by all apps that
	use uIP, such as the planned telnet and FTP clients. Also it means
	we don't have to bloat each program with a complete config UI
	for things like the local/peer/DNS IP addresses.

	In fact the SETUP utility might resolve IP addresses for us, so
	each app might not need its own copy of the resolver... though
	that's maybe a bit clumsy to use for FTP, since you generally want
	to easily be able to connect to arbitrary servers (unlike, say, IRC
	where you usually use the same server all the time). Perhaps SETUP
	could resolve arbitrary hostnames, then write them to a /etc/hosts
	type of file. Parsing /etc/hosts entries is fairly lightweight,
	could be done in all the client programs...

	...or maybe what should happen is that there should be a RESOLV
	program that can resolve hostnames, and read/write them in text
	form to a HOSTS.TXT file... and also parses the HOSTS file and
	stores the hostname strings and 4-byte IP addresses in a fixed
	area of memory. The app programs could just look up hostnames in
	this memory area, without having to open & parse HOSTS themselves.
	cc65 programs by default start at $2E00. The Bob-verter driver ends
	around $2200 (other R: drivers should be comparable), which gives
	us around 3K of unallocated memory. Assuming the average hostname/IP
	pair is 50 bytes, that'd give us about 60 of them. The RESOLV program
	would stop the user from adding more hostnames when there's no space
	for them (and let them delete some to make room, maybe commenting them
	out so they remain in the HOSTS file for future uncommenting).

	Ultimately there should be a master AUTORUN.SYS file with a menu
	that runs the other sub-apps (or lets you exit to DUP). Menu would
	have options like "DNS", "IRC", "FTP", "Telnet", and "Network
	Setup". Of course I need to write a program loader, there's no
	exec* functions in cc65! The Network Setup option would let you
	specify the R: driver you want to load, so it wouldn't have to
	be prepended to any of the executables, and would be auto-loaded
	the first time you try to run any of the apps that need it. It'd
	be nice if the main menu program had at least a few DOS functions
	as well (delete/copy/rename/lock/unlock, format too?)

	Probably, FujiChat's 1.0 release will be as a standalone IRC client,
	with the current config UI stuff as-is. The other stuff would be
	released months from now, under the label "FujiNet" or something, and
	include a new version of FujiChat as one of several apps.

	The final distibution disk will need at least some documentation in
	Atari-readable text format, plus a doc-reader program.

	All the subprograms need to end with RTS to the menu program, which
	shold be small and stay resident in addresses not touched by cc65
	(I've got 128 bytes each in the cassette buffer and page 5, also
	256 more in page 6. Should be easy to squeeze into 512 bytes, no?)

	So a disk directory would contain:

	DOS.SYS, DUP.SYS
	AUTORUN.SYS - the main menu and subprogram loader
	BOBVERT.XEX - the Bob-verter R: driver
	A850.XEX - the 850 R: driver (from DOS 2.0S I guess)
	MIO.XEX, BLACKBOX.XEX, etc, drivers for other serial ports
	HOSTS.TXT - the /etc/hosts file, human-readable and -editable
	README.TXT - docs
	MORE.XEX - doc reader (should handle ASCII or ATASCII EOLs)
	SETUP.XEX - set up global network settings (local/remote/dns IPs, also
	            things like screen colors, key-repeat rate, etc).
	DNS.XEX - lookup hostnames, maintain and parse HOSTS.TXT
	FTP.XEX, TELNET.XEX, IRC.XEX, PING.XEX - the apps
	FTP.CFG, TELNET.CFG, IRC.CFG, etc - app-specific settings
	FUJINET.CFG - global settings
	...if there's room on the disk, a small text editor might be nice
	to include.

	The whole thing should be distributed as a bootable, single-density
  	DOS disk (undecided which DOS, either 2.0S or MyDOS 4.5).

	Alternatively, all the XEX files might be named COM, and the whole
	thing could run under a CLI DOS like Sparta (if it'll actually
	run). The apps could take optional CLI arguments, also... and the
	AUTORUN.SYS would actually be unnecessary!

 */

#define BUF_SIZE 256
#define OUTBUF_SIZE 768
#define NICKLEN 20
#define HOSTLEN 64

/* fuji_conf_t is stored in the config file verbatim.
	Don't forget to update CONF_VERSION when changing this struct!
	Also, the first member should never, ever change, and the 2nd
	(version) should only ever be set to CONF_VERSION (which can be
	changed)

	TODO:
	char alt_nick[NICKLEN+1];
 */
typedef struct {
	char signature[2];
	u16_t version;

	/* uIP settings: */
	uip_ipaddr_t local_ip;
	uip_ipaddr_t peer_ip;
	uip_ipaddr_t resolver_ip;
	char baud;

	/* IRC settings: */
	char server[HOSTLEN+1];
	u16_t server_port;
	char nick[NICKLEN+1];
	char real_name[NICKLEN+1];
	char channel[NICKLEN+1];

	/* UI settings: */
	char bg_color;
	char fg_color;
	u16_t ui_flags; /* bitwise OR of UIFLAG_* constants */
} fuji_conf_t;

fuji_conf_t config;

/* Whether or not to ring the bell on receiving private message */
#define UIFLAG_MSGBELL 0x8000

/* Whether or not to show [PING,PONG] */
#define UIFLAG_SHOWPING 0x4000

/* Visual bell (screen flash) instead of audible */
#define UIFLAG_VBELL 0x2000

/* Hide (don't display) the server MOTD */
/* TODO: implement */
#define UIFLAG_HIDEMOTD 0x1000

/* Disable bells entirely */
#define UIFLAG_NOBELL 0x0800

char os_version[10] = "XL_XE";
char channel[BUF_SIZE];
char input_buf[BUF_SIZE];
char chan_msg_buf[BUF_SIZE * 2];
char output_buf[OUTBUF_SIZE + 1];
char last_msg_nick[NICKLEN+1] = "";

int input_buf_len = 0;
int output_buf_len = 0;

char config_valid = 0;
char done = 0;
char connected = 0;
char joined_channel = 0; /* true if we're in a channel */
char nick_registered = 0;
char vbell_active = 0;

static void get_line(char *buf, unsigned char len);
static void handle_keystroke(void);
static void handle_command(void);
static void get_config(void);
static void config_menu(void);
static void send_server_cmd(char *cmd, char *arg);
static void bell();
void redraw_user_buffer();

/* ATASCII characters */
#define A_EOL 0x9b
#define A_TAB 0x7f
#define A_BS 0x7e
#define A_CLR 0x7d
#define A_BEL 0xfd
#define A_DEL 0x9c

/* plain ASCII characters */
#define NL 0x0a
#define CR 0x0d
#define TAB 0x09

/* TODO: move these to an external file or otherwise come up with a way
	to reuse the memory */
#define SERVER_LIST_LEN 8
static char *servers[][2] = {
	/*
	{ "82.165.139.95", "irc.tobug.net (US)" },
	{ "63.243.153.235", "irc.carterroadband.com (US)" },
	{ "209.133.9.109", "ircd.eyearesee.org (US)" },
	{ "89.238.135.210", "london.eyearesee.org (EU)" },
	{ "193.23.141.104", "hungary.eyearesee.org (EU)" },
	*/
	{ "na.newnet.net", "NewNet (North America)" },
	{ "eu.newnet.net", "NewNet (Europe)" },
	{ "us.undernet.org", "Undernet (US)" },
	{ "eu.undernet.org", "Undernet (Europe)" },
	{ "irc.freenode.org", "FreeNode" },
	{ "kubrick.freenode.net", "FreeNode (US)" },
	{ "irc.efnet.org", "Eris Free Net" },
	{ "82.165.139.95", "irc.tobug.net (NewNet, US)" },
};

/* uIP API stuff */
struct timer nick_reg_timer; /* ../uip/timer.h */
struct timer vbell_timer;
struct telnet_state *tstate; /* ../apps/telnet.h */

/*---------------------------------------------------------------------------*/
int main(void) {
	char c;
	int i;
	uip_ipaddr_t ipaddr;
	struct timer periodic_timer;
	char cmdbuf[HOSTLEN+1];

	c = get_ostype() & AT_OS_TYPE_MAIN;
	if(c == AT_OS_400800) {
		strcpy(os_version, "800");
	} else if(c == AT_OS_1200XL) {
		strcpy(os_version, "1200XL");
	}

	cursor(1); /* don't let conio hide the cursor */

	/* set screen colors to white-on-black initially. They may be
		reset by the user's config file, once it's loaded */
	POKE(710, 0);
	POKE(709, 12);

	putchar(A_CLR);
	puts(BANNER);
	putchar('\n');

	get_config();

	while(1) {
		done = 0;
		rs232dev_close();

		POKE(710, config.bg_color);
		POKE(709, config.fg_color);

		printf("Server name/IP, [C]onfig, or e[X]it\n");
		printf("[%s]: ", config.server);
		fflush(stdout);
		get_line(cmdbuf, HOSTLEN);

		if(strcasecmp(cmdbuf, "x") == 0) {
			exit(0);
		} else if(strcasecmp(cmdbuf, "c") == 0) {
			config_menu();
			continue;
		} else if(*cmdbuf) {
			strcpy(config.server, cmdbuf);
		}
		/* else, use the existing config.server */

		timer_set(&periodic_timer, CLOCK_SECOND / 2);

		rs232dev_init(config.baud);
		uip_init();

		uip_sethostaddr(config.local_ip);
		uip_setdraddr(config.peer_ip);

		/* can I use 255.255.255.252 here? Does it matter? */
		uip_ipaddr(ipaddr, 255,255,255,0);
		uip_setnetmask(ipaddr);

		if(uiplib_ipaddrconv(config.server, (unsigned char*)&ipaddr)) {
			(void)uip_connect(&ipaddr, htons(config.server_port));
		} else {
			resolv_init();
			resolv_conf(config.resolver_ip);
			resolv_query(config.server);
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

			if(vbell_active && timer_expired(&vbell_timer)) {
				POKE(710, config.bg_color);
				vbell_active = 0;
			}

			/* Call the keyboard handler if the user pressed a key. Note that
				we're using stdio for printing, but conio for reading, which
				works on the Atari but may not work on all other cc65 platforms */
			if(kbhit()) {
				handle_keystroke();
			}

			/* Automatically register the nick and userhost after being
				connected for a couple seconds. The parameters for the USER
				command are ignored by the server (at least the NewNet servers) */
			if(connected && !nick_registered && timer_expired(&nick_reg_timer)) {
				puts("Registering nick");
				sprintf(input_buf, "NICK %s%cUSER %s %s %s :%s%c",
						config.nick, NL, config.nick, SELF,
						config.server, config.real_name, NL);
				telnet_send(tstate, input_buf, strlen(input_buf));
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
	return 0;
}

static void bell() {
	if(config.ui_flags & UIFLAG_NOBELL)
		return;

	if(config.ui_flags & UIFLAG_VBELL) {
		vbell_active = 1;
		timer_set(&vbell_timer, CLOCK_SECOND / 10);
		POKE(710, config.bg_color + 8);
	} else {
		putchar(A_BEL);
	}
}

/* The err_* functions will probably support "visual bell" (screen flashing)
	as an option one day. */
static void err_missing_arg(void) {
	puts("Command requires argument");
	bell();
}

static void err_no_channel(void) {
	puts("You are not in a channel (use /join #channel)");
	bell();
}

static void err_already_joined(void) {
	puts("You are already in a channel (use /part to leave)");
	bell();
}

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
		putchar(0x1c);
		putchar(A_DEL);
		putchar(A_DEL);
		redraw_user_buffer();
	} else do {
		putchar(A_BS);
	} while(--bs);
}

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
		keyclick noise (but ignore its return value)

		Make Break act like delete-line

	*/
	c = cgetc();

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
						putchar(input_buf[i] | 0x80);
				}
				return;

			default:
				break;
		}
	}

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
			putchar(c);
		} else {
			bell();
		}

		return;
	} else if(c == A_DEL || c == 0x15) {
		/* shift-backspace or ^U (delete line) */
		input_buf_len = 0;
		putchar(A_DEL);
		return;
	} else if(c == 0x17) {
		/* ^W (delete word) */
		del_last_word();
		return;
	}

	/* Echo keystroke */
	putchar(c | 0x80); /* inverse video for now */

	if(c == A_EOL) {
		c = NL;
		send_buf = 1;
	} else if(c == A_TAB) {
		c = TAB;
	}

	/* Store keystroke in input buffer */
	input_buf[input_buf_len++] = c;

	/* If line too long, ring the "margin" bell */
	if(input_buf_len > BUF_SIZE - 1)
		if(!send_buf) {
			bell();
			putchar(A_BS);
			input_buf_len--;
		}

	/* If we've got a complete line of input and user has pressed Return,
		send it to the server */
	if(send_buf) {
		input_buf[input_buf_len] = '\0';

		if(*input_buf == '/') {
			handle_command();
		} else if(joined_channel) {
			sprintf(chan_msg_buf, "PRIVMSG %s :%s", channel, input_buf);
			telnet_send(tstate, chan_msg_buf, strlen(chan_msg_buf));
		} else {
			err_no_channel();
		}

		input_buf_len = 0;
	}
}

static void send_server_cmd(char *cmd, char *arg) {
	if(arg) {
		sprintf(chan_msg_buf, "%s %s%c", cmd, arg, NL);
	} else {
		sprintf(chan_msg_buf, "%s%c", cmd, NL);
	}

	telnet_send(tstate, chan_msg_buf, strlen(chan_msg_buf));
}

static void send_ctcp_version(char *arg) {
	sprintf(chan_msg_buf, "PRIVMSG %s %cVERSION%c%c",
			arg, 1, 1, NL);
	telnet_send(tstate, chan_msg_buf, strlen(chan_msg_buf));
}

static void send_ctcp_ping(char *arg) {
	sprintf(chan_msg_buf, "PRIVMSG %s %cPING %c%c",
			arg, 1, 1, NL);
	telnet_send(tstate, chan_msg_buf, strlen(chan_msg_buf));
}

static void do_me(char *arg) { /* Do me, baby! */
	sprintf(chan_msg_buf, "PRIVMSG %s :%cACTION %s%c%c",
			channel, 1, arg, 1, NL);
	telnet_send(tstate, chan_msg_buf, strlen(chan_msg_buf));
}

static void handle_command(void) {
	char *cmd = input_buf + 1, *cend;
	char *arg = NULL, *p = cmd;

	/* convert command word to uppercase */
	while((*p != NL) && (*p != ' ')) {
		*p = toupper(*p);
		++p;
	}

	cend = p;

	/* set arg pointer, nul-terminate arg, if present */
	/* (otherwise, arg remains NULL) */
	if(*p != NL) {
		arg = p;
		/* skip extra whitespace */
		while(*arg == ' ')
			++arg;
		p = arg + 1;
		while(*p && (*p != NL))
			++p;
		*p = '\0';
	}

	/* nul-terminate cmd */
	*cend = '\0';

	/* See if it's a command that needs client-side help */
	if(strcmp(cmd, "ME") == 0) {
		if(!joined_channel)
			err_no_channel();
		else if(!arg)
			err_missing_arg();
		else
			do_me(arg);
	} else if(strcmp(cmd, "MSG") == 0 || strcmp(cmd, "M") == 0) {
		if(!arg)
			err_missing_arg();
		else
			send_server_cmd("PRIVMSG", arg);
	} else if(strcmp(cmd, "JOIN") == 0 || strcmp(cmd, "J") == 0) {
		if(joined_channel) {
			err_already_joined();
		} else {
			joined_channel = 1;
			strcpy(channel, arg);
			send_server_cmd("JOIN", arg);
		}
	} else if(strcmp(cmd, "PART") == 0) {
		if(!joined_channel) {
			err_no_channel();
		} else {
			joined_channel = 0;
			send_server_cmd("PART", channel);
		}
	} else if(strcmp(cmd, "VERSION") == 0 || strcmp(cmd, "VER") == 0) {
		if(!arg)
			err_missing_arg();
		else
			send_ctcp_version(arg);
	} else if(strcmp(cmd, "PING") == 0) {
		if(!arg)
			err_missing_arg();
		else
			send_ctcp_ping(arg);
	} else if(strcmp(cmd, "QUOTE") == 0) {
		if(!arg)
			err_missing_arg();
		else
			send_server_cmd(arg, NULL);
	} else {
		/* Anything we don't recognize as a client command, we pass
			directly to the server. */
		send_server_cmd(cmd, arg);
	}
}

/* The telnet_* functions are uIP application callbacks. */
void telnet_connected(struct telnet_state *s) {
	/* puts("Connected to host, press START to disconnect"); */
	puts("Connected to server");
	tstate = s;
	s->text = NULL;
	s->textlen = 0;
	timer_set(&nick_reg_timer, CLOCK_SECOND);
	connected = 1;
}

void telnet_closed(struct telnet_state *s) {
	puts("Connection closed");
	uip_close();
	done = 1;
}

void telnet_sent(struct telnet_state *s) {
}

void telnet_aborted(struct telnet_state *s) {
	puts("Connection aborted");
	uip_abort();
	done = 1;
}

void telnet_timedout(struct telnet_state *s) {
	puts("Connection timed out");
	uip_abort();
	done = 1;
}

void do_pong() {
	char *p = chan_msg_buf;

	memcpy(chan_msg_buf, output_buf, output_buf_len);
	chan_msg_buf[1] = 'O';
	while(*p) {
		if(*p == A_EOL) *p = NL;
		p++;
	}

	telnet_send(tstate, chan_msg_buf, output_buf_len);
	if(config.ui_flags & UIFLAG_SHOWPING)
		puts("[PING,PONG]");
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

	/* no ! in nick means a server message or something, just print as-is */
	if(!bang) {
		fputs(output_buf, stdout);
		fflush(stdout);
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
		if(strcasecmp(chan, config.nick) != 0) {
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
				*bang = '\0';
				output_buf[output_buf_len - 2] = '\0';
				sprintf(chan_msg_buf, "NOTICE %s :\x01PING %s\x01%c",
						nick, msg + 6, 0x0a);
				telnet_send(tstate, chan_msg_buf, strlen(chan_msg_buf));
				printf("* CTCP PING from %s\n", nick);
			} else if(memcmp(msg, "\x01" "VERSION\x01", 9) == 0) {
				nick++;
				*bang = '\0';
				sprintf(chan_msg_buf, "NOTICE %s :\x01VERSION "
						VERSION_REPLY " - running on "
						"an Atari %s\x01%c",
						nick, os_version, 0x0a);
				telnet_send(tstate, chan_msg_buf, strlen(chan_msg_buf));
				printf("* CTCP VERSION from %s\n", nick);
			} else {
				printf("MSG: %s %s", nick, msg);
				if(config.ui_flags & UIFLAG_MSGBELL)
					bell();
				*bang = '\0';
				strcpy(last_msg_nick, nick + 1);
			}
		}
	} else {
		if(*msg) {
			printf("%s %s %s %s", nick, cmd, chan, msg);
		} else {
			printf("%s %s %s", nick, cmd, chan);
		}
	}

	fflush(stdout);
}

void del_user_buffer() {
	/* delete user edit buffer */
	if(input_buf_len) {
		putchar(A_DEL);
		if(input_buf_len > 119) {
			putchar(0x1c); /* up arrow */
			putchar(A_DEL);
		}
	}
}

void redraw_user_buffer() {
	int tlen;
	static char redraw_buf[BUF_SIZE];
	/* reprint user edit buffer */
	if(input_buf_len) {
		tlen = input_buf_len - 1;
		do {
			redraw_buf[tlen] = input_buf[tlen] | 0x80;
		} while(tlen--);
		redraw_buf[input_buf_len] = '\0';
		fputs(redraw_buf, stdout);
		fflush(stdout);
	}
}

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
		} else if(c == '{') {
			c = '[' | 0x80;
		} else if(c == '}') {
			c = ']' | 0x80;
		} else if(c == '~') {
			c = '^' | 0x80;
		} else if(c == 0x60) { // backtick
			c = 0xa7; // inverse quote
		}

		output_buf[output_buf_len++] = c;
		if(output_buf_len >= OUTBUF_SIZE - 1) {
			puts("[buffer overflow]\xfd");
			buf_done = 1;
			break;
		}

		if(buf_done) {
			del_user_buffer();
			output_buf[output_buf_len] = '\0';

			if(output_buf_len > 4 && (memcmp(output_buf, "PING", 4) == 0)) {
				do_pong();
			} else if(output_buf[0] == ':') {
				do_msg();
			} else {
				fputs(output_buf, stdout);
				fflush(stdout);
			}

			redraw_user_buffer();
			output_buf_len = 0;
			buf_done = 0;
		}
	}
}

/* Load config file, offer the user a chance to change the config */
static void get_config(void) {
	// char filename[20];
	FILE *f = fopen(DEFAULT_CONF_FILE, "rb");

	puts("Loading config from " DEFAULT_CONF_FILE);

	if(f) {
		config_valid =
			fread(&config, 1, sizeof(fuji_conf_t), f) == sizeof(fuji_conf_t);
		fclose(f);
		if(!config_valid)
			puts("Config file is wrong size");
	} else {
		puts("No config file found");
	}

	if(config_valid) {
		if(memcmp(config.signature, CONF_SIGNATURE, 2) != 0) {
			puts("Not a valid config file");
			config_valid = 0;
		} else if(config.version != CONF_VERSION) {
			puts("Config is for the wrong version of FujiChat, ignoring");
			config_valid = 0;
		} else {
			puts("Loaded OK");
			/*
			printf("Loaded OK. Use defaults [Y/n]? ");
			fflush(stdout);
			fgets(input_buf, 10, stdin);
			if(*input_buf == 'N' || *input_buf == 'n')
				config_menu();
				*/
		}
	}

	if(!config_valid)
		config_menu();
}

static void save_config(void) {
	FILE *f = fopen(DEFAULT_CONF_FILE, "wb");
	if(f) {
		fwrite(&config, sizeof(fuji_conf_t), 1, f);
		fclose(f);
		puts("Config saved to " DEFAULT_CONF_FILE);
	}
}

static void get_conf_color(char *name, char *color, char dflt) {
	char buf[5];

	printf("%s color [%d]: ", name, dflt);
	fgets(buf, 5, stdin);

	if(!(*buf >= '0' && *buf <= '9'))
		*color = dflt;
	else
		*color = atoi(buf);
}

static void get_line(char *buf, unsigned char len) {
	fgets(buf, len, stdin);
	while(*buf) {
		if(*buf == '\n')
			*buf = '\0';
		++buf;
	}
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

static void get_ip_addr(char *prompt, char *dflt, uip_ipaddr_t *addr) {
	char ok;
	char buf[HOSTLEN+1];

	do {
		printf("%s IP address [%s]: ", prompt, dflt);
		fflush(stdout);
		get_line(buf, HOSTLEN);
		if(!*buf) strcpy(buf, dflt);
		ok = uiplib_ipaddrconv(buf, (unsigned char*)addr);
		if(!ok) bell();
	} while(!ok);
}

static char *format_ip(uip_ipaddr_t *ip) {
	static char buf[20];
	u16_t *ipaddr = (u16_t *)ip;
	sprintf(buf, "%d.%d.%d.%d",
			htons(ipaddr[0]) >> 8,
			htons(ipaddr[0]) & 0xff,
			htons(ipaddr[1]) >> 8,
			htons(ipaddr[1]) & 0xff);
	return buf;
}

void set_default_config(void) {
	uip_ipaddr(&(config.local_ip), 192,168,0,2);
	uip_ipaddr(&(config.peer_ip), 192,168,0,1);
	uip_ipaddr(&(config.resolver_ip), 192,168,0,1);
	strcpy(config.server, servers[0][0]);
	strcpy(config.nick, DEFAULT_NICK);
	strcpy(config.real_name, "FujiChat User");
	config.server_port = 6667;
	config.ui_flags = 0;
	config.bg_color = 0xc0; /* dark green */
	config.fg_color = 0x0c;
	config.baud = RS_BAUD_4800;

	config_valid = 1;
}

int baud_values[] = { 1200, 2400, 4800, 9600 };
char baud_bytes[] = { RS_BAUD_1200, RS_BAUD_2400, RS_BAUD_4800, RS_BAUD_9600 };
char max_baud = sizeof(baud_bytes);

static void get_baudrate(void) {
	char i, buf[5];

	for(i=0; i<max_baud; ++i)
		if(config.baud == baud_bytes[i])
			printf("[%d]=%d  ", i+1, baud_values[i]);
		else
			printf("%d=%d  ", i+1, baud_values[i]);

	fputs("Baud rate: ", stdout);
	fflush(stdout);

	do {
		get_line(buf, 4);
		if(!*buf)
			return;

		i = atoi(buf);
	} while (i < 1 || i > max_baud);
	config.baud = baud_bytes[--i];
}

static void config_menu(void) {
	char i, buf[HOSTLEN+1], ok = 0;
	unsigned int new_value;

	strcpy(config.signature, CONF_SIGNATURE);
	config.version = CONF_VERSION;

	if(!config_valid)
		set_default_config();

	do {
		get_baudrate();
		get_ip_addr("Local", format_ip(&(config.local_ip)), &(config.local_ip));
		get_ip_addr("Peer", format_ip(&(config.peer_ip)), &(config.peer_ip));
		get_ip_addr("DNS Server", format_ip(&(config.resolver_ip)), &(config.resolver_ip));

		puts("Server list:");
		for(i=0; i<SERVER_LIST_LEN; ++i) {
			printf("%d: %s\n   %s\n",
					i + 1,
					servers[i][0],
					servers[i][1]);
		}

		printf("\nEnter # or any server host/IP [%s]: ", config.server);
		fflush(stdout);
		fgets(buf, HOSTLEN, stdin);

		if(buf[0] == '\n') {
			buf[0] = '1';
			buf[1] = '\n';
		}

		if(buf[0] >= '1' && buf[0] <= (SERVER_LIST_LEN + '0') && buf[1] == '\n') {
			strcpy(buf, servers[buf[0] - '1'][0]);
		}

		for(i=0; buf[i] && (buf[i] != '\n'); ++i)
			;

		buf[i] = '\0';
		strcpy(config.server, buf);

		printf("Server port [%d]: ", config.server_port);
		get_line(buf, 10);
		new_value = atoi(buf);
		if(new_value) config.server_port = new_value;

		printf("Your nickname [%s]: ", config.nick);
		get_line(buf, NICKLEN);
		if(*buf) strcpy(config.nick, buf);

		printf("Your 'real' name [%s]: ", config.real_name);
		get_line(buf, NICKLEN);
		if(*buf) strcpy(config.real_name, buf);

		// if(!*(config.nick)) strcpy(config.nick, DEFAULT_NICK);
		/* TODO: scan for invalid chars */

		get_conf_color("Background", &(config.bg_color), config.bg_color);
		get_conf_color("Foreground", &(config.fg_color), config.fg_color);

		new_value =
			(get_yesno("Disable bell", config.ui_flags & UIFLAG_NOBELL) ? UIFLAG_NOBELL : 0);

		if(!(config.ui_flags & UIFLAG_NOBELL)) {
			new_value |=
				(get_yesno("Visual bell", config.ui_flags & UIFLAG_VBELL) ? UIFLAG_VBELL : 0);

			new_value |=
				(get_yesno("Bell on msg", config.ui_flags & UIFLAG_MSGBELL) ? UIFLAG_MSGBELL : 0);
		};

		new_value |=
			(get_yesno("Show ping/pong", config.ui_flags & UIFLAG_SHOWPING) ? UIFLAG_SHOWPING : 0);

		config.ui_flags = new_value;

		ok = get_yesno("Is this correct", 1);
	} while(!ok);

	if(get_yesno("Save this config", 1))
		save_config();
}

/*---------------------------------------------------------------------------*/

/* uIP app callback for the resolver */
void resolv_found(char *name, u16_t *ipaddr) {
	if(ipaddr == NULL) {
		printf("Host '%s' not found.\n", name);
		done = 1;
	} else {
		printf("Found name '%s' = %d.%d.%d.%d\n", name,
				htons(ipaddr[0]) >> 8,
				htons(ipaddr[0]) & 0xff,
				htons(ipaddr[1]) >> 8,
				htons(ipaddr[1]) & 0xff);
		if(!connected)
			(void)uip_connect((uip_ipaddr_t *)ipaddr, htons(config.server_port));
	}
}
