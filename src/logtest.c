#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* 0x100 * 32 = 8K (but we likely won't get that much) */
#define LOG_PAGE_SIZE 0x20
#define LOG_MAX_PAGES 16
#define LOG_SCROLL_PAGES 2

static char *log_buffer = NULL;
static char *log_buf_end = NULL;
static char *log_buf_current = NULL;
static int log_pages = LOG_MAX_PAGES;

// static char last_log_nick[NICKLEN + 1];

static void alloc_log_buffer() {
	log_buffer = malloc(LOG_PAGE_SIZE * log_pages);
	log_buf_end = (char *)(log_buffer + log_pages * LOG_PAGE_SIZE - 1);
	log_buf_end[1] = 0xaa;
	log_buf_current = log_buffer;
}

static void log_msg(char *msg) {
	int len = strlen(msg);
	if( (log_buf_current + len + 1) > log_buf_end ) {
		memmove(log_buffer, log_buffer + (LOG_SCROLL_PAGES * LOG_PAGE_SIZE), (log_pages - LOG_SCROLL_PAGES) * LOG_PAGE_SIZE);
		log_buf_current -= (LOG_SCROLL_PAGES * LOG_PAGE_SIZE);
	}

	memcpy(log_buf_current, msg, len);
	log_buf_current += (len + 1);
	*log_buf_current = '\0';
}

static void dump_log_stats(void) {
	printf("start: %x, end: %x, cur: %x\n", log_buffer, log_buf_end, log_buf_current);
}

void main(void) {
	char buf[100];
	int i;

	alloc_log_buffer();
	dump_log_stats();

	for(i=0; i<24; i++) {
		sprintf(buf, ":--------------------Iteration #%d\n", i);
		log_msg(buf);
		dump_log_stats();
	}
HANG: goto HANG;
}
