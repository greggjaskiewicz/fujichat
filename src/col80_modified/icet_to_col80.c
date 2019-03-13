#include <stdio.h>

int main(int argc, char **argv) {
	int c, d, byte = 0;
	char out[8];

	do {
		c = getchar();
		if(byte == 16) {
			for(byte = 0; byte < 8; ++byte)
				putchar(out[byte]);

			byte = 0;
		}

		if(c != EOF) {
			if(byte < 8) {
				out[byte] = (c & 0xf0);
				++byte;
			} else if(byte < 16) {
				out[byte - 8] |= (c & 0x0f);
				++byte;
			}
		}
	} while(c != EOF);
}
