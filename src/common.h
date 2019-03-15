#ifndef COMMON_H
#define COMMON_H

#include "fujichat.h"

/* common functions, shared by fujichat.c and fujiconf.c.
	Don't put anything here unless it really is shared! */

/* This stays resident */
extern fuji_conf_t *config;

/* config-related */
char get_config(void);
char config_is_valid(void);
void set_default_config(void);

/* UI-related */
char __fastcall__ fuji_cgetc(void);
void __fastcall__ fuji_putchar(char); /* fujiput.s */
void get_line(char *buf, unsigned char len);
void disable_break(void);

#endif
