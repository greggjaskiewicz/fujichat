
/* Dead simple keyboard buffer mechanism

	Call keybuf_init() at startup

	During busy loops, periodically call keybuf_poll_kbd(),
	which will buffer keystrokes as they're pressed.

	When ready to read the keyboard, call keybuf_cgetc()
	Return value is 0 for no key pressed and no keys in buffer,
	or else the ASCII code of the next key.

	Also, can call keybuf_is_empty() to find out whether the
	buffer's got anything in it.

	It would be possible to have the OS periodically call
	keybuf_poll_kbd() for us, using one of the countdown timers.
	For now, not going to do this, as it adds complexity and
	fujichat only has 2 or 3 places that count as busy
	loops.
 */

void __fastcall__ keybuf_init(void);
void __fastcall__ keybuf_poll_kbd(void);
char __fastcall__ keybuf_is_empty(void);
char __fastcall__ keybuf_cgetc(void);
