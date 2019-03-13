
void __fastcall__ fuji_putchar(char);

void main(void) {
	char msg[] = "Test\n";
	char i;

	for(i=0; i<5; ++i)
		fuji_putchar(msg[i]);

HANG: goto HANG;
}
