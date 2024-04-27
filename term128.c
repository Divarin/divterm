#include <stdio.h>
#include <serial.h>
#include <cbm.h>
#include <conio.h>
#include <stdlib.h>
#include <serial.h>
#include <peekpoke.h>
#include <c128.h>
#include <accelerator.h>
#include "term.h"

int currentbaud;
int currentvideo;
char key;
int status;
//unsigned char string[100];
int err;

int main(void) {
	
	currentbaud = 4;

	setVideo(0);
	
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
	int bufferindex;
	int i;
	char *buffer;
	
	pause=0;
	bufferindex=0;
	i=0;
	buffer = (char*)(malloc(BUFFER_SIZE));
	
	printf(" -- DivTerm Ready --\nF1 : Baud, F3 : Video --\n");
	
	while (1)
    {
        char chr;

        if (kbhit())
        {
            chr = cgetc();
			if (pause) {
				POKE(0xd020,7);
				for (i=0; i < bufferindex; i++) {
					putchar(*(buffer+i));
				}
				bufferindex=0;
				pause = 0;
				POKE(0xd020,11);
				continue;
			}
			switch (chr)
			{
				case 3:
					if (!pause) {
						pause = 1;
						POKE(0xd020,2);
					}
					continue;
				case 10:
					continue;
				case CH_F1:
					setBaud((currentbaud + 1) % 7);
					continue;
				case CH_F2: break;
				case CH_F3:
					setVideo((currentvideo + 1) % 2); // change to % 3 to allow dual video mode
					continue;
				case CH_F4: break;
				case CH_F5:
					printf("\n\nFree memory: %u\n\n",_heapmemavail());
					continue;
				case CH_F6: break;
				case CH_F7: break;
				case CH_F8: break;
			}

			ser_put(chr);
        }

        if (ser_get (&chr) == SER_ERR_OK && chr != 10) {
			if (pause) {
				if (bufferindex < BUFFER_SIZE)
					*(buffer+bufferindex++)=chr;
			} else {
				putchar(chr);	
			}
        }
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