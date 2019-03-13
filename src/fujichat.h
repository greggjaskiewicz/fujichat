#ifndef FUJICHAT_H
#define FUJICHAT_H

#include "uip.h"
#include "aexec.h"

/* Program name (plain ASCII for network use) */
#define SELF "FujiChat"
/* inverse video version, used for local prompts: */
#define SELF_INV "\xc6\xf5\xea\xe9\xc3\xe8\xe1\xf4"

#define VERSION "0.5_pre4"
#define BANNER SELF_INV " v" VERSION
#define VERSION_REPLY SELF " v" VERSION

#define DEFAULT_NICK SELF

// #define CONF_SIGNATURE "\x03\x0e"
#define CONF_SIGNATURE_LO 0x03
#define CONF_SIGNATURE_HI 0x0e
#define CONF_VERSION 5

#define BUF_SIZE 256
#define OUTBUF_SIZE 512
#define NICKLEN 20
#define HOSTLEN 64

/* fuji_conf_t is stored in the config file verbatim.
	Don't forget to update CONF_VERSION when changing this struct!
	Also, the first member should never, ever change, and the 2nd
	(version) should only ever be set to CONF_VERSION (which can be
	changed)

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
	char alt_nick[NICKLEN+1];
	char real_name[NICKLEN+1];
	char channel[NICKLEN+1];

	/* UI settings: */
	char bg_color;
	char fg_color;
	u16_t ui_flags; /* bitwise OR of UIFLAG_* constants */

	/* Other stuff: */
	char timezone; /* for later, when we use /TIME to set the clock */
} fuji_conf_t;

/* Chunk of RAM that holds our config data. Setup leaves the config
	here for FujiChat to find. */
#define CONFIG_ADDRESS 0x500

#define ARGTYPE_NONE     0
#define ARGTYPE_REQUIRED 1
#define ARGTYPE_OPT      2

typedef struct {
	char *cmd;          /* e.g. "ME" or "PING" */
	char arg_type;      /* one of ARGTYPE_* constants */
	void (*handler)(void);
} fuji_cmd_t;

extern fuji_cmd_t cmd_list[];
extern char *serv_msg_buf;
extern int serv_msg_buf_len;
extern char *cmd_arg;
extern char channel[];
extern struct telnet_state *tstate;
extern char joined_channel;
extern char *input_buf;

void send_serv_msg_buf(void);
void send_server_cmd(char *cmd, char *arg);
void send_server_cmd_2arg(char *cmd, char *arg);
void handle_command(void);
void bell();

/* Filenames */
#define DEFAULT_CONF_FILE "D:FUJICHAT.CFG"
#define MENU_FILENAME "D:FUJIMENU.COM"
#define SETUP_FILENAME "D:FUJICONF.COM"
#define IRC_FILENAME "D:FUJICHAT.COM"
// #define IRC_LOADER_FILENAME "D:LOADCHAT.COM"
#define SERIAL_DRIVER_FILENAME "D:DEFAULT.SER"
#define DEFAULT_SERIAL_DRIVER "D:BOBVERT.SER"

/* Whether or not to ring the bell on receiving private message */
#define UIFLAG_MSGBELL 0x8000

/* Whether or not to show [PING,PONG] */
#define UIFLAG_SHOWPING 0x4000

/* Visual bell (screen flash) instead of audible */
#define UIFLAG_VBELL 0x2000

/* Hide (don't display) the server MOTD */
#define UIFLAG_HIDEMOTD 0x1000

/* Disable bells entirely */
#define UIFLAG_NOBELL 0x0800

/* Auto-away when ATRACT mode kicks in */
#define UIFLAG_ATRACT_AWAY 0x400


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

/* cc65's rs232.h doesn't define this for Atari: */
#ifndef RS_BAUD_19200
#define RS_BAUD_19200 0x0f
#endif

#endif
