#include <stdio.h>
//#include <c128.h>
#include <serial.h>
#include <cbm.h>
#include <conio.h>
#include <stdlib.h>
#include <serial.h>
#include <peekpoke.h>
//#include <accelerator.h>
#include "term.h"

int currentbaud;
int currentvideo;
char key;
int status;
unsigned char string[100];


int err;

int main(void) {
	currentbaud = 4;

	setVideo(0);
	
	/* clear screen, set colors */
	printf("%c%c",147,5);
	POKE(0xd020,11);
	POKE(0xd021,0);

	err=ser_load_driver("c64-swlink.ser");
	if (err!=SER_ERR_OK) printf("Error loading driver!\n");

	/* open port */
	err=ser_open(&p9600);
	if (err!=SER_ERR_OK) printf("Error opening port!\n");
	printf("Terminal Ready!\n\n");

	while (1)
		term();

	printf("Closing Terminal\n\n");
	cgetc();
	
	ser_close();
	exit(0);
	
	return EXIT_SUCCESS;
}

void term() {
	int cursor;
	
	cursor = 1;
	putchar(18); // rev on
	putchar(32); // space
	putchar(146); // rev off
	
	while (1) {
		if (kbhit ()) {
			key=cgetc();
			switch (key)
			{
				case CH_F1:
					setBaud((currentbaud + 1) % 7);
					break;
				case CH_F2: break;
				case CH_F3:
					setVideo((currentvideo + 1) % 3);	
					break;
				case CH_F4: break;
				case CH_F5:
					printf("\n\nFree memory: %u\n\n",_heapmemavail());
					break;
				case CH_F6: break;
				case CH_F7: break;
				case CH_F8: break;
				default:
					ser_put(key);
					break;
			}
		}

		do {
			status=ser_get(&key);
			if (status==SER_ERR_OK && key != 10) {
				if (cursor) {
					cursor = 0;
					//putchar(157); // cursor left
					switch(key) {
						case 13:
							//putchar(32); // space over cursor block before printing the newline.
							break;
						case 157:
						case 145:
						case 29:
						case 17:
							//putchar(32); // space over cursor block before moving cursor.
							//putchar(157); // cursor left
							break;
					}
				}
				
				if (key == '"')
					cputc(key); // force vic write
				else
					putchar(key);
					
				if (currentvideo==2) {
					// dual-screen mode, also put char on 80 column display.
					setVideoMode(5);
					putchar(key);
					setVideoMode(0);
				}
			}
		} while (status != SER_ERR_NO_DATA);
		
		if (!cursor) {
			cursor = 1;
			//putchar(18); // rev on
			//putchar(32); // space
			//putchar(146); // rev off
		}
	}
}

void setVideo(int video) {
	currentvideo=video;
	printf("setting video mode: %i\n", currentvideo);
	
	switch (currentvideo) {
		case 1:
			// 80 col VDC
			setVideoMode(5);
			//set_c128_speed(1); 
			break;
		case 2:
			// 40 & 80 together
			setVideoMode(0);
			//set_c128_speed(0);
			break;
		default:
			// 40 col VIC
			setVideoMode(0);
			//set_c128_speed(0); 
			break;
	}
}

void setVideoMode(int videoMode) {
	// TODO
}

void setBaud(int baud) {
	struct ser_params p;
	
	switch (baud) {
		case 1: 
			p = p1200; 
			printf("Setting to 1200 baud.\n");
			break;
		case 2: 
			p = p2400; 
			printf("Setting to 2400 baud.\n");
			break;
		case 3: 
			p = p4800; 
			printf("Setting to 4800 baud.\n");
			break;
		case 4: 
			p = p9600; 
			printf("Setting to 9600 baud.\n");
			break;
		case 5: 
			p = p19200; 
			printf("Setting to 19200 baud.\n");
			break;
		case 6: 
			p = p38400; 
			printf("Setting to 38400 baud.\n");
			break;
		default: 
			p = p300; 
			printf("Setting to 300 baud.\n");
			break;
	}
	
	currentbaud = baud;
	
	ser_close();
	err=ser_open(&p);
	if (err!=SER_ERR_OK) printf("Error opening port!\n");
	printf("Terminal Ready!\n\n");
}