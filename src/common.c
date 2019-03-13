#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <rs232.h>
#include "uip.h"
#include "common.h"
#include "aexec.h"
#include "fujichat.h"

int __fastcall__ (*atari_exec_p)(char *) = (int __fastcall__ (*)(char *))0x600;
fuji_conf_t *config = (fuji_conf_t *)CONFIG_ADDRESS;

void disable_break(void) {
	asm("lda $10"); /* POKMSK */
	asm("and #$7f");
	asm("sta $10"); /* POKMSK */
	asm("sta $D20E"); /* IRQEN */
}

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
u16_t local_htons(u16_t val) {
	__AX__ = val;
	asm("sta tmp1");
	asm("txa");
	asm("ldx tmp1");
}

/* helper for fuji_cgetc */
void __fastcall__ call_keybdv(void) {
	asm("lda $E420+5"); /* KEYBDV */
	asm("pha");
	asm("lda	$E420+4");
	asm("pha");
	asm("lda	#12");
	asm("sta	$2A"); /* ICAX1Z */
}

/* replacement for cgetc() that doesn't include cursor enable/disable
	stuff (otherwise copied from cc65's cgetc.s) */
char __fastcall__ fuji_cgetc(void) {
	asm("jsr	_call_keybdv");
	asm("ldx	#0");
}


#if 0
char get_config(void) {
	char config_valid = 0;
	FILE *f = fopen(DEFAULT_CONF_FILE, "rb");

	puts("Loading config from " DEFAULT_CONF_FILE);

	if(f) {
		config_valid =
			fread(config, 1, sizeof(fuji_conf_t), f) == sizeof(fuji_conf_t);
		fclose(f);
		if(!config_valid)
			puts("Config file is wrong size");
	} else {
		puts("No config file found");
	}

	if(config_valid) {
		if(!config_is_valid()) {
			puts("Invalid or outdated config file");
			config_valid = 0;
		} else {
			puts("Loaded OK");
		}
	}

	return config_valid;
}

#else
/* using open() read() close() instead of fopen() fread() fclose()
	is a big win: save us 438 bytes! */
char get_config(void) {
	char config_valid = 0;
	int f = open(DEFAULT_CONF_FILE, O_RDONLY);

	puts("Loading config from " DEFAULT_CONF_FILE);

	if(f >= 0) {
		config_valid =
			(read(f, config, sizeof(fuji_conf_t)) == sizeof(fuji_conf_t));
		close(f);
		if(!config_valid)
			puts("Config file is wrong size");
	} else {
		puts("No config file found");
	}

	if(config_valid) {
		if(!config_is_valid()) {
			puts("Invalid or outdated config file");
			config_valid = 0;
		} else {
			puts("Loaded OK");
		}
	}

	return config_valid;
}
#endif

char config_is_valid(void) {
	// return( (memcmp(config->signature, CONF_SIGNATURE, 2) == 0) && (config->version == CONF_VERSION) );
	return
		*((char *)CONFIG_ADDRESS) == CONF_SIGNATURE_LO &&
		*(char *)(CONFIG_ADDRESS+1) == CONF_SIGNATURE_HI &&
		*(char *)(CONFIG_ADDRESS+2) == CONF_VERSION;
}

void set_default_config(void) {
	puts("Using built-in defaults");
	/*
		// FIXME: why does this end up always 0.0.0.0 now?
		// has something to do with macro expansion. What a mess.
	uip_ipaddr(&(config->local_ip), 192,168,0,2);
	uip_ipaddr(&(config->peer_ip), 192,168,0,1);
	uip_ipaddr(&(config->resolver_ip), 192,168,0,1);
	*/

	config->local_ip[0] = 0xa8c0;
	config->local_ip[1] = 0x0200;

	config->peer_ip[0] = 0xa8c0;
	config->peer_ip[1] = 0x0100;

	config->resolver_ip[0] = 0xa8c0;
	config->resolver_ip[1] = 0x0100;

	strcpy(config->server, "na.newnet.net");
	strcpy(config->nick, DEFAULT_NICK);
	strcpy(config->real_name, "FujiChat User");
	config->server_port = 6667;
	config->ui_flags = UIFLAG_HIDEMOTD;
	config->bg_color = 0xc0; /* dark green */
	config->fg_color = 0x0c;
	config->baud = RS_BAUD_4800;

	*(char *)(CONFIG_ADDRESS) = CONF_SIGNATURE_LO;
	*(char *)(CONFIG_ADDRESS+1) = CONF_SIGNATURE_HI;
	*(char *)(CONFIG_ADDRESS+2) = CONF_VERSION;

	// config_valid = 1;
}

static char ipbuf[20];
/*
static char *fmt = "%d.%d.%d.%d";
*/
char * format_ip(uip_ipaddr_t *ip) {

	u16_t *ipaddr = (u16_t *)ip;
	sprintf(ipbuf, "%d.%d.%d.%d",
			local_htons(ipaddr[0]) >> 8,
			local_htons(ipaddr[0]) & 0xff,
			local_htons(ipaddr[1]) >> 8,
			local_htons(ipaddr[1]) & 0xff);

	return ipbuf;

	/*
	asm("	sta ptr1");
	asm("	stx ptr1+1");

	asm("	lda #<_ipbuf");
	asm("	ldx #>_ipbuf");
	asm("	jsr pushax");

	asm("	lda #<_fmt");
	asm("	ldx #>_fmt");
	asm("	jsr pushax");

	asm("	ldy #0");
	asm("	lda (ptr1),y");
	asm("	jsr pusha0");

	asm("	ldy #1");
	asm("	lda (ptr1),y");
	asm("	jsr pusha0");

	asm("	ldy #2");
	asm("	lda (ptr1),y");
	asm("	jsr pusha0");

	asm("	ldy #3");
	asm("	lda (ptr1),y");
	asm("	jsr pusha0");

	asm("	ldy #$0c");
	asm("	jsr _sprintf");

	asm("	lda #<_ipbuf");
	asm("	ldx #>_ipbuf");
	*/
}

void get_line(char *buf, unsigned char len) {
	asm("ldy #$00");
	asm("lda (sp),y");
	asm("sta $e2");

	asm("ldy #$02");
	asm("jsr ldaxysp");
	asm("sta $e0");
	asm("stx $e0+1");

	asm("@loop: lda _stdin");
	asm("ldx _stdin+1");
	asm("jsr _fgetc");

	asm("cmp #$9B");
	asm("beq @out");

	asm("ldy #$00");
	asm("sta ($e0),y");
	asm("inc $e0");
	asm("bne @noinc");
	asm("inc $e0+1");
	asm("@noinc:");

	asm("dec $e2");
	asm("bne @loop");

	asm("@out:");
	asm("ldy #$00");
	asm("tya");
	asm("sta ($e0),y");

	/*
		// C implementation is 80 bytes, asm is 50
	fgets(buf, len, stdin);
	while(*buf) {
		if(*buf == '\n')
			*buf = '\0';
		++buf;
	}
	*/
}

