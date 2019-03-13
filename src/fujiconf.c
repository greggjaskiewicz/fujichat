#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <rs232.h>
#include <errno.h>
#include <peekpoke.h>
#include "uip.h"
#include "uiplib.h"
#include "fujichat.h"
#include "common.h"

/* TODO: move these to an external file or otherwise come up with a way
	to reuse the memory */
#define SERVER_LIST_LEN (sizeof(servers) / sizeof(servers[0]))
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

// char rs232_already_loaded = 0;

int baud_values[] = { 1200, 2400, 4800, 9600, 19200 };
char baud_bytes[] = { RS_BAUD_1200, RS_BAUD_2400, RS_BAUD_4800, RS_BAUD_9600, RS_BAUD_19200 };
char max_baud = sizeof(baud_bytes);

static void get_conf_color(char *name, char *color, char dflt);
static char get_yesno(char *prompt, char dflt);
void save_config(void);

static void bell() {
	putchar(A_BEL);
}

static void get_baudrate(void) {
	char i, buf[5];

	putchar('\n');
	for(i=0; i<max_baud; ++i)
		if(config->baud == baud_bytes[i])
			printf("[%d] %d\n", i+1, baud_values[i]);
		else
			printf(" %d. %d\n", i+1, baud_values[i]);

	fputs("\nBaud rate: ", stdout);
	fflush(stdout);

	do {
		get_line(buf, 4);
		if(!*buf)
			return;

		i = atoi(buf);
	} while (i < 1 || i > max_baud);
	config->baud = baud_bytes[--i];
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

#if 0
char scan_hatabs(char dev) {
	int entry;

	/* 0x031A is HATABS from OS equates */
	for(entry = 0x031A; entry < 0x033F; entry += 3)
		if(PEEK(entry) == dev) return 1;

	return 0;
}

int copy_file(char *dst, char *src) {
	// THIS FUNCTION IS BROKEN!
	static char buf[256];
	int i;
	FILE *from, *to;

	if( !(from = fopen(src, "rb")) ) {
		perror(src);
		i = errno;
		goto done;
	}

	if( !(to = fopen(dst, "wb")) ) {
		perror(dst);
		i = errno;
		goto done;
	}

	while( (i = fread(buf, 1, 256, from)) )
		fwrite(buf, 1, i, to);

	if(feof(from))
		i = 0;
	else
		i = errno;

done:
	if(from) fclose(from);
	if(to) fclose(to);
	return i;
}
#endif

static void config_menu(void) {
	char i, buf[HOSTLEN+1], ok = 0;
	unsigned int new_value;

	// strcpy(config->signature, CONF_SIGNATURE);
	((char *)config->signature)[0] = CONF_SIGNATURE_LO;
	((char *)config->signature)[1] = CONF_SIGNATURE_HI;
	config->version = CONF_VERSION;

	if(!config_is_valid())
		set_default_config();

	do {
		puts("\n==> Serial Port Settings\n");

		if(get_yesno("Change RS232 driver", 0)) {
			atari_exec("D:MAKEAUTO.COM");
		}

		get_baudrate();

		/* TODO: choose which Rn: device to use (e.g. on the 850),
			turn HW flow control on/off (buried in cc65 rs232 lib code) */

		puts("\n==> Network Settings\n");
		get_ip_addr("Local", format_ip(&(config->local_ip)), &(config->local_ip));
		get_ip_addr("Peer", format_ip(&(config->peer_ip)), &(config->peer_ip));
		get_ip_addr("DNS Server", format_ip(&(config->resolver_ip)), &(config->resolver_ip));

		puts("\n==> IRC Settings\n");
		puts("Server list:");
		for(i=0; i<SERVER_LIST_LEN; ++i) {
			printf("%d: %s\n   %s\n",
					i + 1,
					servers[i][0],
					servers[i][1]);
		}

		printf("\nEnter # or any server host/IP [%s]: ", config->server);
		fflush(stdout);
		fgets(buf, HOSTLEN, stdin);

		if(*buf != '\n') {
			if(buf[0] >= '1' && buf[0] <= (SERVER_LIST_LEN + '0') && buf[1] == '\n') {
				strcpy(buf, servers[buf[0] - '1'][0]);
			}

			for(i=0; buf[i] && (buf[i] != '\n'); ++i)
				;

			buf[i] = '\0';
			strcpy(config->server, buf);
		}

		printf("Server port [%d]: ", config->server_port);
		get_line(buf, 10);
		new_value = atoi(buf);
		if(new_value) config->server_port = new_value;

		printf("Your nickname [%s]: ", config->nick);
		get_line(buf, NICKLEN);
		if(*buf) strcpy(config->nick, buf);

		printf("Your 'real' name [%s]: ", config->real_name);
		get_line(buf, NICKLEN);
		if(*buf) strcpy(config->real_name, buf);

		do {
			printf("Autojoin channel (or 0 for none)\n[%s]: ", config->channel);
			get_line(buf, NICKLEN);

			switch(*buf) {
				case '\0':
					ok = 1;
					break;

				case '#':
				case '&':
					strcpy(config->channel, buf);
					ok = 1;
					break;

				case '0':
					*(config->channel) = '\0';
					ok = 1;
					break;

				default:
					// user-friendly:
					*(config->channel) = '#';
					strcpy((config->channel) + 1, buf);
					ok = 1;
					break;

					// user-hostile:
					/*
					puts("IRC channel names must begin with # or &");
					bell();
					ok = 0;
					*/
			}

			/* TODO: scan for illegal chars, set ok=0 */
		} while(!ok);


		puts("\n==> User Interface Settings\n");
		do {
			get_conf_color("Background", &(config->bg_color), config->bg_color);
			get_conf_color("Foreground", &(config->fg_color), config->fg_color);
			ok = ((config->bg_color & 0x0e) != (config->fg_color & 0x0e));
			if(!ok)
				puts("These colors would make your text\n"
						"impossible to see. Try again.\n");
		} while(!ok);

		new_value =
			(get_yesno("Disable bell", config->ui_flags & UIFLAG_NOBELL) ? UIFLAG_NOBELL : 0);

		if(!(new_value & UIFLAG_NOBELL)) {
			new_value |=
				(get_yesno("Visual bell", config->ui_flags & UIFLAG_VBELL) ? UIFLAG_VBELL : 0);

			new_value |=
				(get_yesno("Bell on msg", config->ui_flags & UIFLAG_MSGBELL) ? UIFLAG_MSGBELL : 0);
		};

		new_value |=
			(get_yesno("Show ping/pong", config->ui_flags & UIFLAG_SHOWPING) ? UIFLAG_SHOWPING : 0);

		new_value |=
			(get_yesno("Hide MOTD", config->ui_flags & UIFLAG_HIDEMOTD) ? UIFLAG_HIDEMOTD : 0);

		config->ui_flags = new_value;

		puts("\n==> Config complete\n");
		ok = get_yesno("Is this correct", 1);
	} while(!ok);

	if(get_yesno("Save this config", 1))
		save_config();

	putchar('\n');
}

void save_config(void) {
	FILE *f = fopen(DEFAULT_CONF_FILE, "wb");
	if(f) {
		fwrite(config, sizeof(fuji_conf_t), 1, f);
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

void reboot(void) {
	asm("jmp $e477");
}

void main(void) {
	char *next_file, buf[2];
	int ret;

	disable_break();

	putchar(125);
	puts(BANNER " Setup\n");
	get_config();

	if(!config_is_valid())
		set_default_config();

	config_menu();

	/*
	if(rs232_already_loaded) {
		puts("\nAn RS232 driver is already resident.\n"
				"You must reboot to use the new driver.\n");
		if(get_yesno("Reboot now", 1))
			reboot();
	}
	*/

	if(get_yesno("Start FujiChat now", 1)) {
		// next_file = IRC_LOADER_FILENAME;
		next_file = IRC_FILENAME;
	} else {
		next_file = MENU_FILENAME;
	}

	printf("\nLoading %s...\n", next_file);
	ret = atari_exec(next_file);

	printf("Error %d!\n\xfd", ret);
	puts("Press Enter to exit to DOS");
	get_line(buf, 2);
}
