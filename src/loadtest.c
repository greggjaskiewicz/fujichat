#include <stdio.h>

void __fastcall__ (*atari_exec_p)(char *) = (void __fastcall__ (*)(char *))0x600;
#define atari_exec(x) ((*atari_exec_p)(x))

int main(int, char **) {
	puts("Loading D:RVERT.COM");
	atari_exec("D:RVERT.COM");
	puts("Loading D:FUJICHAT.RAW");
	atari_exec("D:FUJICHAT.RAW");
	puts("Should never get here!");
}
