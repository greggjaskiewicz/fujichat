
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <peekpoke.h>
#include "fujichat.h"
#include "features.h"
#include "common.h"

void cmd_join(void);
void cmd_msg(void);
void cmd_nick(void);
void cmd_part(void);
void cmd_ping(void);
// void cmd_quit(void);
void cmd_quote(void);
void cmd_ver(void);
void do_me(void);
#ifdef FEAT_COLOR_COMMAND
void cmd_fgcolor(void);
void cmd_bgcolor(void);
#endif

fuji_cmd_t cmd_list[] = {
	{ "JOIN", ARGTYPE_OPT, cmd_join },
	{ "J", ARGTYPE_OPT, cmd_join },
	{ "MSG", ARGTYPE_REQUIRED, cmd_msg },
	{ "M", ARGTYPE_REQUIRED, cmd_msg },
	{ "NICK", ARGTYPE_REQUIRED, cmd_nick },
	{ "PART", ARGTYPE_NONE, cmd_part },
	{ "PING", ARGTYPE_REQUIRED, cmd_ping },
	// { "QUIT", ARGTYPE_OPT, cmd_quit },
	{ "QUOTE", ARGTYPE_REQUIRED, cmd_quote },
	{ "VER", ARGTYPE_REQUIRED, cmd_ver },
	{ "VERSION", ARGTYPE_REQUIRED, cmd_ver },
	{ "ME", ARGTYPE_REQUIRED, do_me },
#ifdef FEAT_COLOR_COMMAND
	{ "FGCOLOR", ARGTYPE_REQUIRED, cmd_fgcolor },
	{ "BGCOLOR", ARGTYPE_REQUIRED, cmd_bgcolor },
#endif
};

#define CMD_LIST_SIZE (sizeof(cmd_list) / sizeof(fuji_cmd_t))

char *cmd_arg;

static void errmsg(char *msg) {
	puts(msg);
	bell();
}

static void err_missing_arg(void) {
	errmsg("Command requires argument");
}

static void err_arg_not_allowed(void) {
	errmsg("No argument allowed");
}

static void err_already_joined(void) {
	errmsg("You are already in a channel (use /part to leave)");
}


void do_me(void) {
	serv_msg_buf_len = sprintf(serv_msg_buf, "PRIVMSG %s :%cACTION %s%c%c",
			channel, 1, cmd_arg, 1, NL);
	send_serv_msg_buf();
}

void cmd_ver(void) {
	serv_msg_buf_len = sprintf(serv_msg_buf, "PRIVMSG %s %cVERSION%c%c",
			cmd_arg, 1, 1, NL);
	send_serv_msg_buf();
}

void cmd_ping(void) {
	serv_msg_buf_len = sprintf(serv_msg_buf, "PRIVMSG %s %cPING %03d %03d %03d%c%c",
			cmd_arg, 1, PEEK(20), PEEK(19), PEEK(18), 1, NL);
	send_serv_msg_buf();
}

void cmd_join(void) {
	if(joined_channel) {
		if(cmd_arg)
			err_already_joined();
		else
			send_server_cmd("JOIN", channel);
	} else {
		if(cmd_arg) {
			joined_channel = 1;
			strcpy(channel, cmd_arg);
			send_server_cmd("JOIN", cmd_arg);
		} else {
			err_missing_arg();
		}
	}
}

void cmd_part(void) {
	joined_channel = 0;
	send_server_cmd("PART", channel);
}

void cmd_msg(void) {
	send_server_cmd("PRIVMSG", cmd_arg);
}

void cmd_quote(void) {
	send_server_cmd(cmd_arg, NULL);
}

void cmd_nick(void) {
	strcpy(config->nick, cmd_arg);
	send_server_cmd("NICK", cmd_arg);
	printf("> You are now known as %s\n", config->nick);
}

#ifdef FEAT_COLOR_COMMAND
static void do_color(u16_t reg) {
	u16_t c;

	if(cmd_arg[0] >= '0' && cmd_arg[0] <= '9') {
		c = atoi(cmd_arg);
		if(c < 0x100) {
			POKE(reg, c);
			return;
		}
	}

	puts("Bad color (range 0-255)");
	bell();
}

void cmd_fgcolor(void) {
	do_color(709);
}

void cmd_bgcolor(void) {
	do_color(710);
}

#endif

void handle_command(void) {
	char i, *cend;
	char *cmd = input_buf + 1;
	char *p = cmd;

	cmd_arg = NULL;

	/* convert command word to uppercase */
	while((*p != NL) && (*p != ' ')) {
		*p = toupper(*p);
		++p;
	}

	cend = p;

	/* set cmd_arg pointer, nul-terminate cmd_arg, if present */
	/* (otherwise, cmd_arg remains NULL) */
	if(*p != NL) {
		cmd_arg = p;
		/* skip extra whitespace */
		while(*cmd_arg == ' ')
			++cmd_arg;
		p = cmd_arg + 1;
		while(*p && (*p != NL))
			++p;
		*p = '\0';
	}

	/* nul-terminate cmd */
	*cend = '\0';

	for(i=0; i<CMD_LIST_SIZE; ++i) {
		if(strcmp(cmd, cmd_list[i].cmd) != 0)
			continue;

		switch(cmd_list[i].arg_type) {
			case ARGTYPE_REQUIRED:
				if(!cmd_arg) {
					err_missing_arg();
					return;
				}
				break;

			case ARGTYPE_NONE:
				if(cmd_arg) {
					err_arg_not_allowed();
					return;
				}
				return;

			default:
				break;
		}

		(cmd_list[i].handler)();
		return;
	}

	/* no idea what this is, send it as-is */
	send_server_cmd(cmd, cmd_arg);
}
