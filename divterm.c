#include <stdio.h>
#include <serial.h>
#include <cbm.h>
#include <conio.h>
#include <stdlib.h>
#include <serial.h>
#include <peekpoke.h>
#include <c128.h>
#include <accelerator.h>
#include "divterm.h"

int currentbaud;
int currentvideo;
int currentemu;
char key;
int status;
int err;

int main(void) {
	
	currentbaud = 4;
	currentvideo = 0;
	currentemu = EMU_CBM;
	
	/* clear screen, set VIC screen colors colors */
	printf("%c%c",147,5);
	POKE(0xd020,11);
	POKE(0xd021,0);

	/* workaround for intermittent c128 crashing issues 
	   caused by a conflict between code in c128-swlink.ser
	   and the 128's keyboar scanning interrupt.  
	   Thank you OldWoman37 from Lemon64 forums for finding */
	POKE(0x318, 0x33);
	POKE(0x319, 0xff);
	
	err=ser_load_driver("c128-swlink.ser");
	if (err!=SER_ERR_OK) printf("Error loading driver!\n");

	/* open port */
	err=ser_open(&p9600);
	if (err!=SER_ERR_OK) printf("Error opening port!\n");

	term();

	// since term() contains an infinite loop we never should get here
	
	printf("Closing Terminal\n\n"); // but it'd be nice to know if we did
	cgetc();
	
	ser_close();
	exit(0);
	
	return EXIT_SUCCESS;
}

void term() {
	int pause;
	int i;
	int cursor;
	char *bs; // buffer start
	char *rp; // buffer read pointer
	char *wp; // buffer write pointer
	char *tmp;
	int scrolltoend;
	
	pause=0;
	i=0;
	
	bs = (char*)(malloc(BUFFER_SIZE));
	// fill the scrollback buffer with dots
	//for (i=0; i < BUFFER_SIZE; i++)
	//	*(bs+i)='.';
		
	wp = bs; // initialize write pointer to start of buffer
	cursor = 1;
	
	printf(" -- DivTerm Ready --\n");
	showHelp();
	
	while (1)
    {
        char chr;

        if (kbhit())
        {
            chr = cgetc();
			if (pause) {
				scrolltoend = SCROLL_AMT;
				switch (chr) {
					case CH_UP:
						// scroll-back
						for (i=0; i < 1000+SCROLL_AMT; i++) {
							rp--;
							if (rp < bs) rp = BUFFER_END-1; // wrap to end
							if (rp == wp) {
								rp++;
								if (rp >= BUFFER_END) rp=bs; // wrap to start
								break;
							}
						}
						putchar(CH_CLR);
						scrolltoend += 500;
						// fall through to scroll forward
					case CH_DOWN:
						// scroll-forward
						for (i=0; i < scrolltoend && rp != wp; i++) {
							rp++;
							if (rp >= BUFFER_END) rp = bs;
							if (rp != wp && *rp != CH_CLR && *rp != CH_HOME) putchar(*rp);
						}
						if (rp == wp) {
							// unpause
							pause = 0;
							POKE(0xd020,11);
						}
						break;
					default:
						// spit out remainder of buffer and leave pause mode
						POKE(0xd020,7); // VIC border yellow
						while (rp != wp) {
							putchar(*rp);
							rp++;
							if (rp >= BUFFER_END)
								rp = bs; // wrap around to start
						}
						pause = 0;
						POKE(0xd020,11);
						break;
				}
				continue;
			}
			switch (chr)
			{
				case 3:
					if (!pause) {
						// activate pause mode
						pause = 1;
						rp = wp;
						POKE(0xd020,2); // VIC border red
					}
					continue;
				case 10:
					continue;
				case CH_F1:
					ClearCursor
					showHelp();
					continue;
				case CH_F2: break;
				case CH_F3:
					ClearCursor
					setBaud((currentbaud + 1) % 7);
					continue;
				case CH_F4: break;
				case CH_F5:
					ClearCursor
					setVideo((currentvideo + 1) % 2); // change to % 3 to allow dual video mode
					continue;
				case CH_F6: break;
				case CH_F7:
					ClearCursor
					setEmu((currentemu + 1) % NUM_EMUS);
					continue;
				case CH_F8:
					ClearCursor
					printf("\n\nFree memory: %u\n\n",_heapmemavail());
					continue;
			}
			
			if (currentemu != EMU_CBM)
				chr = translateOut(chr);
			
			ser_put(chr);
        }

        while (ser_get (&chr) == SER_ERR_OK && chr != 10) {
			if (currentemu != EMU_CBM)
				chr = translateIn(chr);
			
			tmp = wp+1;
			if (tmp >= BUFFER_END)
				tmp = bs; // wrap around to start
			if (!pause || tmp != rp) {
				wp = tmp;
				*wp = chr;
			}
			
			if (pause) continue;
			
			if (cursor) {
				switch (chr) {
					case 13:
					case CH_UP:
					case CH_DOWN:
					case CH_LEFT:
					case CH_RIGHT:
					case CH_HOME:
					case CH_CLR:
						ClearCursor
						break;
				}
			}
			if (chr == CH_QUOTE) {
				putchar(CH_QUOTE);
				putchar(CH_BACKSPACE);					
			}
			putchar(chr);
			cursor = 0;
        }
		
		cursor = 1;
		putchar(CH_CURSOR);
		putchar(CH_LEFT);
    }
}
	
void setVideo(int video) {
	currentvideo=video;
	
	switch (currentvideo) {
		case 1:
			// 80 col VDC
			printf("\n\nSetting Video Mode: VDC\n");
			set_c128_speed(1);
			videomode(5);
			printf("\n\nSetting Video Mode: VDC\n");
			break;
		case 2:
			// 40 & 80 together
			printf("\n\nSetting Video Mode: VIC & VDC\n");
			break;
		default:
			// 40 col VIC
			printf("\n\nSetting Video Mode: VIC\n");
			set_c128_speed(0);
			videomode(1);
			printf("\n\nSetting Video Mode: VIC\n");			
			break;
	}
}

void setBaud(int baud) {
	struct ser_params p;
	
	switch (baud) {
		case 1: 
			p = p1200; 
			printf("\n\nSetting to 1200 baud.\n\n");
			break;
		case 2: 
			p = p2400; 
			printf("\n\nSetting to 2400 baud.\n\n");
			break;
		case 3: 
			p = p4800; 
			printf("\n\nSetting to 4800 baud.\n\n");
			break;
		case 4: 
			p = p9600; 
			printf("\n\nSetting to 9600 baud.\n\n");
			break;
		case 5: 
			p = p19200; 
			printf("\n\nSetting to 19200 baud.\n\n");
			break;
		case 6: 
			p = p38400; 
			printf("\n\nSetting to 38400 baud.\n\n");
			break;
		default: 
			p = p300; 
			printf("\n\nSetting to 300 baud.\n\n");
			break;
	}
	
	currentbaud = baud;
	
	ser_close();
	err=ser_open(&p);
	if (err!=SER_ERR_OK) printf("Error opening port!\n");
}

void setEmu(int emu) {
	switch (emu) {
		case EMU_ASCII:
			currentemu = EMU_ASCII;
			printf("\n\nASCII emulation\n\n");
			break;
		default:
			currentemu = EMU_CBM;
			printf("\n\nCBM/PETSCII emulation\n\n");
			break;
	}
}

void showHelp() {	
	printf("\n -- DivTerm --\n");
	printf("F1: This Menu\n");
	printf("F3: Set Baud\n");
	printf("F5: 40/80 Col\n");
	printf("F7: Emulation\n\n");
}

char translateIn(char c) {
	if (currentemu == EMU_ASCII) {
		// uppercase letters
		if (c >= 65 && c <= 90) return c+128;
		
		// lowercase letters
		if (c >= 97 && c <= 122) return 65+(c-97);
		
		// backspace
		if (c == 8) return 20;
	}
	
	return c;
}

char translateOut(char c) {	
	if (currentemu == EMU_ASCII) {
		// uppercase letters
		if (c >= 193 && c <= 218) return 65 + (c-193);
		
		// lowercase letters
		if (c >= 65 && c <= 90) return 97 + (c-65);
		
		// backspace
		if (c == 20) return 8;
	}
	
	return c;
}