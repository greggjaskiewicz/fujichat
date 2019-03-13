#ifndef FEATURES_H
#define FEATURES_H


// #define FEAT_COL64_HACK
// #define FEAT_COL80_HACK

#ifdef FEAT_COL64_HACK
#define COLUMNS 64
#define FEAT_COL80_HACK
#endif

#if defined(FEAT_TINY) && defined(FEAT_COL80_HACK)
#error "You may not define both FEAT_TINY and FEAT_COL80_HACK"
#endif

#ifdef FEAT_TINY

#define FEAT_LOW_RAM_BUFFERS
#undef FEAT_DYNAMIC_VERSION
#undef FEAT_VISUAL_BELL
#undef FEAT_CURSOR_CONTROLS
#undef FEAT_KEYBOARD_MACROS
#undef FEAT_TRAFFIC_INDICATOR
#undef FEAT_ATRACT_AWAY
#undef FEAT_KEYBOARD_BUFFER
#undef FEAT_COLOR_COMMAND

#elif defined(FEAT_COL80_HACK)

#define FEAT_LOW_RAM_BUFFERS
#define FEAT_VISUAL_BELL
#define FEAT_KEYBOARD_MACROS
// #define FEAT_TRAFFIC_INDICATOR
#define FEAT_KEYBOARD_BUFFER
#define FEAT_COLOR_COMMAND

/* COL80 hack uses 9K of high RAM, so let's disable
	some bells & whistles. */
#undef FEAT_DYNAMIC_VERSION
#undef FEAT_ATRACT_AWAY

/* Do not enable cursor controls with COL80 hack, because they don't work
	anyway (arrows and deletes not implemented in the driver). Not needed
	anyway, fujichat.c contains code that does the same thing when COL80
	is enabled (slower though) */
#undef FEAT_CURSOR_CONTROLS

#else
/* Removable features. Compiling with everything turned off
	(except FEAT_LOW_RAM_BUFFERS!) saves us around 1322 bytes at
	runtime (including data/bss) */

/* Keep our bigger buffers in low memory, where possible. Saves
	space in the DATA segment, allowing MEMTOP to be set lower
	(to make room for more scrollback, or an 80-col driver, etc) */
#define FEAT_LOW_RAM_BUFFERS

/* Make CTCP version response say "Atari 8-bit" instead of calling
	get_ostype(). Saves 179 bytes in executable. */
#define FEAT_DYNAMIC_VERSION

/* Support visual bell. Saves 110 bytes in exe when undef'd. */
// TODO: make fujiconf not ask about visual bell, too
#define FEAT_VISUAL_BELL

/* Don't use cursor controls to keep from splitting up the user's
	edit buffer when packets come in while he's typing. The 80-column
	version will probably need this disabled.
	Saves 410 bytes in exe. */
#define FEAT_CURSOR_CONTROLS

/* Disable the keyboard macros (^W, ^N, Tab at start of line). Saves
	about 230 bytes in exe. */
#define FEAT_KEYBOARD_MACROS

/* Have the rs232dev code display up/down arrows in top right of
	screen. Accesses screen RAM directly, don't use with 80 cols. */
#define FEAT_TRAFFIC_INDICATOR

/* Set /away on the server when Atari goes into attract mode,
	clear when user presses a key */
#define FEAT_ATRACT_AWAY

/* Experimental logging (backlog, scrollback) support. Expect trouble. */
// TODO: implement!
// #define FEAT_LOGGING

/* Disable the keyboard prebuffering during serial I/O */
#define FEAT_KEYBOARD_BUFFER

/* Support /bgcolor and /fgcolor command in FujiChat (instead of having to go
	back to setup menu to change text colors) */
#define FEAT_COLOR_COMMAND

/* messing around, not functional yet */
// #define FEAT_UNICODE_TEST

/* End of features */

#endif
#endif
