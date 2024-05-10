#include <stdio.h>
#include <serial.h>
#include <cbm.h>
#include <conio.h>
#include <stdlib.h>
#include <serial.h>
#include <peekpoke.h>
#include <c128.h>
#include <accelerator.h>
#include <string.h>
#include "divterm.h"

int currentBaud;
int currentVideo;
int currentEmu;
char key;
int status;
int err;
char ansiBuffer[ANSI_BUFFER_SIZE];
int ansiBufferIndex;
bool isReverse;
char driveNum;

int main(void)
{	
	driveNum = PEEK(4096);
	
	ansiBufferIndex = 0;
	currentBaud = 4;
	currentVideo = 0;
	currentEmu = EMU_CBM;
	isReverse = false;
	
	initGraphics();
	
	/* clear screen, set VIC screen colors colors */
	//printf("%c%c",147,5);
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

void term()
{
	int flags; 
	int i;
	char *bs; // buffer start
	char *rp; // buffer read pointer
	char *wp; // buffer write pointer
	char *tmp;
	int scrollToEnd;
	
	i = 0;
	
	flags = SW_DECODE_ANSI;
	flags |= SW_CURSOR;
	
	ansiBufferIndex = 0;
	
	bs = (char*)(malloc(BUFFER_SIZE));
	
	wp = bs; // initialize write pointer to start of buffer
	
	printf(" -- DivTerm Ready --\n");
	printf(VERSION);
	
	showHelp();
	
	while (true)
    {
        char chr;

        if (kbhit())
        {
            chr = cgetc();
			if (flags & SW_PAUSE)
			{
				scrollToEnd = SCROLL_AMT;
				switch (chr) {
					case CH_UP:
						// scroll-back
						for (i=0; i < 1000+SCROLL_AMT; i++)
						{
							rp--;
							if (rp < bs) rp = BUFFER_END-1; // wrap to end
							if (rp == wp) {
								rp++;
								if (rp >= BUFFER_END) rp=bs; // wrap to start
								break;
							}
						}
						putchar(CH_CLR);
						scrollToEnd += 500;
						// fall through to scroll forward
					case CH_DOWN:
						// scroll-forward
						for (i=0; i < scrollToEnd && rp != wp; i++)
						{
							rp++;
							if (rp >= BUFFER_END) rp = bs;
							if (rp != wp && *rp != CH_CLR && *rp != CH_HOME) putchar(*rp);
						}
						if (rp == wp)
						{
							// unpause
							flags &= ~SW_PAUSE;
							POKE(0xd020,11);
						}
						break;
					case CH_RIGHT:
						// debug scroll-forward
						if (rp != wp) {
							rp++;
							if (rp >= BUFFER_END) rp = bs;
							if (rp != wp && *rp != CH_CLR && *rp != CH_HOME)
							{
								// use alternate display to show byte value
								if (currentVideo == VID_VIC)
									videomode(5);
								else
									videomode(1);
								printf("%i, ", *rp); // print byte as decimal value
								if (currentVideo == VID_VIC)
									videomode(1);
								else
									videomode(5);
								putchar(*rp); // output the character
							}
						}
						if (rp == wp)
						{
							// unpause
							flags &= ~SW_PAUSE;
							POKE(0xd020,11);
						}
						break;
					default:
						// spit out remainder of buffer and leave pause mode
						POKE(0xd020,7); // VIC border yellow
						while (rp != wp)
						{
							putchar(*rp);
							rp++;
							if (rp >= BUFFER_END)
								rp = bs; // wrap around to start
						}
						flags &= ~SW_PAUSE;
						POKE(0xd020,11);
						break;
				}
				continue;
			}
			switch (chr)
			{
				case 3:
					if (!(flags & SW_PAUSE))
					{
						// activate pause mode
						flags |= SW_PAUSE;
						rp = wp;
						POKE(0xd020,2); // VIC border red
					}
					continue;
				case CH_F1:
					ClearCursor;
					showHelp();
					continue;
				case CH_F2:
					if (flags & SW_DECODE_ANSI)
						flags &= ~SW_DECODE_ANSI;
					else
						flags |= SW_DECODE_ANSI;
					ClearCursor;
					printf("\ndecode ansi: %i\n", flags & SW_DECODE_ANSI);
					continue;
				case CH_F3:
					ClearCursor;
					setBaud((currentBaud + 1) % 7);
					continue;
				case CH_F4:
					continue;
				case CH_F5:
					ClearCursor;
					setVideo((currentVideo + 1) % 2, flags & SW_FAST_VDC); // change to % 3 to allow dual video mode
					continue;
				case CH_F6:
					if (flags & SW_FAST_VDC)
						flags &= ~SW_FAST_VDC;
					else
						flags |= SW_FAST_VDC;
					ClearCursor;
					printf("\nturn off VIC when in VDC mode: %i\n", flags & SW_FAST_VDC);
					continue;
				case CH_F7:
					ClearCursor;
					setEmu((currentEmu + 1) % NUM_EMUS);
					continue;
				case CH_F8:
					ClearCursor;
					printf("\n\nFree memory: %u\n\n",_heapmemavail());
					continue;
			}
			
			if (currentEmu != EMU_CBM)
			{
				chr = translateOut(chr);
				if (chr == 13) {
					// in ASCII mode CR (13) is ignored on incoming
					// only LF's are processed and treated like a CR
					// so since the user just pressed a CR we know we
					// won't get an echo back (well we will but it will be ignored)
					// so we'll manually do the echo right now instead.
					ClearCursor;
					putchar(chr);
				}				
			}
			
			ser_put(chr);
        }

        while (ser_get (&chr) == SER_ERR_OK)
		{			
			if (currentEmu == EMU_CBM && (chr == 9 || chr == 10)) continue;
			if (currentEmu == EMU_ASCII)
			{
				if (chr == 13) continue;	
				if (chr == 10) chr = 13;
			}
						
			// handle ANSI codes
			if (currentEmu == EMU_ASCII && flags & SW_DECODE_ANSI)
			{
				if (chr == 27 && !(flags & SW_COLLECTING_ANSI))
				{
					// found start of ansi sequence (escape)
					flags |= SW_COLLECTING_ANSI;
					continue;
				}
				if (flags & SW_COLLECTING_ANSI)
				{
					if (ansiBufferIndex < ANSI_BUFFER_SIZE-1)
						ansiBuffer[ansiBufferIndex++] = chr; // collect ansi sequence
					if ((chr >= 65 && chr <= 90) || (chr >= 97 && chr <= 122))
					{
						// found end of ansi sequence (a letter)
						ansiBuffer[ansiBufferIndex] = 0;
						flags &= ~SW_COLLECTING_ANSI;
						parseAnsi();
						ansiBufferIndex = 0;
					}
					continue;
				}
			}
			
			// any other non PETSCII translation
			// ASCII to PETSCII
			// future: ATASCII?
			if (currentEmu != EMU_CBM)
			{
				chr = translateIn(chr);
				if (!chr) continue;
			}
			
			// put character into buffer
			// (don't buffer CLEAR and HOME characters if in CBM emulation mode)
			if (currentEmu != EMU_CBM || (chr != CH_CLR && chr != CH_HOME))
			{
				tmp = wp+1;
				if (tmp >= BUFFER_END)
					tmp = bs; // wrap around to start
				if (!(flags & SW_PAUSE) || tmp != rp) {
					wp = tmp;
					*wp = chr;
				}
			}
			
			// ok we read the character, possibly translated it, 
			// possibly put it into a temporary ansi sequence
			// and buffered it, but if we're paused we'll stop here
			// because we don't want to show it.
			if (flags & SW_PAUSE) continue;
			
			// if it's a character that's moving the cursor but not overwriting the cursor
			// then we need to manually overwrite the cursor
			if (flags & SW_CURSOR)
			{
				switch (chr) {
					case 13:
					case CH_UP:
					case CH_DOWN:
					case CH_LEFT:
					case CH_RIGHT:
					case CH_HOME:
					case CH_BLACK:
					case CH_RED:
					case CH_GREEN:
					case CH_YELLOW:
					case CH_BLUE:
					case CH_MAGENTA:
					case CH_CYAN:
					case CH_WHITE:
					case CH_GRAY:
					case CH_REV_ON:
					case CH_REV_OFF:
					case CH_SWITCH_UP:
					case CH_SWITCH_DN:						
						ClearCursor;
						break;
				}
			}
			if (chr == CH_QUOTE)
			{
				putchar(CH_QUOTE);
				putchar(CH_BACKSPACE);					
			}
			putchar(chr);
			flags &= ~SW_CURSOR;
        }
		
		flags |= SW_CURSOR;
		putchar(CH_CURSOR);
		putchar(CH_LEFT);
    }
}
	
void setVideo(int video, bool fast)
{
	currentVideo=video;
	
	switch (currentVideo)
	{
		case 1:
			// 80 col VDC
			printf("\n\nSetting Video Mode: VDC\n");
			if (fast) set_c128_speed(1);
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

void setBaud(int baud)
{
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
	
	currentBaud = baud;
	
	ser_close();
	err=ser_open(&p);
	if (err!=SER_ERR_OK) printf("Error opening port!\n");
}

void setEmu(int emu)
{
	switch (emu)
	{
		case EMU_ASCII:
			currentEmu = EMU_ASCII;
			printf("\n\nASCII/ANSI emulation\n\n");
			break;
		default:
			currentEmu = EMU_CBM;
			printf("\n\nPETSCII/CBM emulation\n\n");
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

// when using ASCII mode, takes the incoming character and returns the petscii
// character.  Ones marked (custom) have patterns loaded from "extascii" file 
// in initGraphics().
char translateIn(char c)
{
	if (currentEmu == EMU_ASCII)
	{
		// uppercase letters
		if (c >= 65 && c <= 90) return c+128;
		
		// lowercase letters
		if (c >= 97 && c <= 122) return 65+(c-97);
		
		// backspace
		if (c == 8) return 20;
		
		switch (c) {
			case CH_DOWN:
			//case CH_UP:
			//case CH_LEFT:
			case CH_RIGHT:
			case CH_HOME:
			//case CH_CLR: // 147
			//case CH_BLACK:
			case CH_RED:
			case CH_GREEN:
			//case CH_YELLOW: // 158
			//case CH_BLUE:
			//case CH_MAGENTA: // 156, same as british pound below
			//case CH_CYAN:
			case CH_WHITE:
			//case CH_GRAY: // 151
			case CH_REV_ON:
			case CH_REV_OFF:
			//case CH_SWITCH_UP:
			case CH_SWITCH_DN:
				return 0;
			case 92: return 223; // backslash (custom)
			case 95: return 228; // underscore
			case 128: return 'C';
			case 129: case 150: case 151: case 163: case 230: return 'u';
			case 130: case 136: case 137: case 138: return 'e';
			case 131: case 132: case 133: case 134: case 160: case 166: case 224: return 'a';
			case 135: case 155: return 'c';
			case 139: case 140: case 141: case 161: case 173: return 'i';
			case 142: case 143: return 'A';
			case 144: return 'E';
			case 147: case 148: case 149: case 162: return 'o';
			case 152: return 'y';
			case 153: case 233: case 229: return 'O';
			case 154: return 'U';
			case 157: return 168; // yen (custom)
			case 164: case 252: return 'n';
			case 165: return 'N';
			case 225: return 'B';
			case 156: return 92; // british pound
			case 176: return 165; // thin hash (custom)
			case 177: return 166; // medium hash (custom)
			case 178: return 167; // thick hash (custom)
			case 179: case 186: return 163; // center vertical line (custom)
			case 180: case 181: case 182: case 185: return 179; // ┤
			case 191: case 183: case 184: case 187: case 170: return 174; // ┐
			case 192: case 200: case 211: case 212: return 173; // └
			case 193: case 202: case 207: case 208: return 177; // ┴
			case 194: case 203: case 209: case 210: return 178; // ┬
			case 195: case 198: case 199: case 204: return 171; // ├
			case 196: case 205: return 96; // center horizontal line
			case 197: case 206: case 215: case 216: return 216; // ┼
			case 217: case 188: case 189: case 190: return 189; // ┘
			case 218: case 201: case 213: case 214: case 169: return 176; // ┌
			case 219: return 220; // solid block (custom)
			case 220: return 162; // lower block
			case 221: return 161; // left block
			case 222: return 182; // right block
			case 223: return 184; // upper block
			case 227: return 126; // pi (custom)
			case 249: case 250: return '.';
			case 254: return 190; // ▘
		}
	}
	
	return c;
}

char translateOut(char c)
{	
	if (currentEmu == EMU_ASCII)
	{
		// uppercase letters
		if (c >= 193 && c <= 218) return 65 + (c-193);
		
		// lowercase letters
		if (c >= 65 && c <= 90) return 97 + (c-65);
		
		// backspace
		if (c == 20) return 8;
	}
	
	return c;
}

void parseAnsi()
{	
	switch (ansiBuffer[ansiBufferIndex-1])
	{
		case 109: // 'm'
			parseAnsiColor();
			break;
		case 74: // 'j'
			// clear screen
			if (ansiBuffer[ansiBufferIndex-2] == '2') {
				putchar(CH_CLR);
			}
			break;
		case 70: // 'f'
		case 72: // 'h'
			parseAnsiHome();
			break;
		case 65: // 'a'
			parseAnsiCursor(CH_UP);
			break;
		case 66: // 'b'
			parseAnsiCursor(CH_DOWN);
			break;
		case 67: // 'c'
			parseAnsiCursor(CH_RIGHT);
			break;
		case 68: // 'd'
			parseAnsiCursor(CH_LEFT);
			break;
	}
}
			
void parseAnsiColor()
{
	char* code;
	char* num;
	int n;
	ansiBuffer[ansiBufferIndex-1] = 0; // drop trailing letter
	code = (char*)(ansiBuffer+1); // drop leading '['
	
	num = strtok(code, ";"); // split by ';' to get tokens (returns first token)
	while (num) {
		n = atoi(num);
		if (isReverse && n >= 31 && n <=37)
		{
			// trying to set foreground color while is reversed
			// skip
			num = strtok(0, ";"); // fetch next token
			continue;
		}
		
		if (isReverse && n == 40)
		{
			// explicity setting background color to black,
			// interpret this as turning reverse off.
			putchar(CH_REV_OFF);
			isReverse = false;
		}
		else if (!isReverse && n >= 41 && n <= 47)
		{
			// setting background to a non-black color
			// turn reverse on
			putchar(CH_REV_ON);
			isReverse = true;
		}
		
		switch (n)
		{
			case 0:
				// reset
				if (isReverse) {
					putchar(CH_REV_OFF);
					isReverse=false;
					putchar(CH_WHITE);
				}
				break;
			case 30: putchar(CH_BLACK); break; // black foreground, hopefully we have (or will have) a non-black background
			case 40: break; // black background, handled in IF above, don't touch foreground color
			case 31: case 41: putchar(CH_RED); break;
			case 32: case 42: putchar(CH_GREEN); break;
			case 33: case 43: putchar(CH_YELLOW); break;
			case 34: case 44: putchar(CH_BLUE); break;
			case 35: case 45: putchar(CH_MAGENTA); break;
			case 36: case 46: putchar(CH_CYAN); break;					
			case 37: case 47: putchar(CH_WHITE); break;	
		}		
		num = strtok(0, ";"); // fetch next token
	};
}

void parseAnsiHome()
{
	char* code;
	char* num;
	int n;
	int i;
	ansiBuffer[ansiBufferIndex-1] = 0; // drop trailing letter
	code = (char*)(ansiBuffer+1); // drop leading '['
	
	ClearCursor;
	putchar(CH_HOME);
	
	if (code != "") {
		num = strtok(code, ";");
		n = atoi(num) - 1;
		for (i=0; i < n; i++) 
			putchar(CH_DOWN);
		num = strtok(0, ";");
		n = atoi(num) - 1;
		for (i=0; i < n; i++)
			putchar(CH_RIGHT);
	}
}

void parseAnsiCursor(char direction)
{
	char* num;
	int n;
	int i;
	ansiBuffer[ansiBufferIndex-1] = 0; // drop trailing letter
	num = (char*)(ansiBuffer+1); // drop leading '['
	n = atoi(num);
	
	if (n) ClearCursor;
		
	for (i=0; i < n; i++) {
		putchar(direction);
	}
}

// loads custom characters for ascii & extended ascii characters
// this includes any character that can't be direclty mapped to a petscii character
void initGraphics()
{
	int bytesRead;
	int r;
	int n;
	int i;
	int offset;
	unsigned char buffer[9];
	
	set_c128_speed(1);	
	cbm_open(2, driveNum, 0, "extascii");
	
	do
	{
		bytesRead = cbm_read(2, buffer, 9);
		if (bytesRead < 9)
			break;
		n = buffer[0]; // offset to address where the pattern lives
		
		for (i = n; i <= n+128; i+=128)
		{
			offset=0x1000 + 16*i;
			for (r=1; r < bytesRead; r++) {
				POKE(0xd600, 18);
				while (!(PEEK(0xd600) & 128)) { }
				POKE(0xd601, (0x2000 + offset)/256);

				POKE(0xd600, 19);
				while (!(PEEK(0xd600) & 128)) { }
				POKE(0xd601, (0x2000 + offset)%256);

				POKE(0xd600, 31);
				while(!(PEEK(0xd600) & 128)) { }
				if (i == n)
					POKE(0xd601, buffer[r]); // write byte pattern for non-reverse character
				else
					POKE(0xd601, ~buffer[r]); // write byte pattern for reverse character

				offset++;
			}
		}
	} while (1);
	
	set_c128_speed(0);
	cbm_close(2);
	
	// start writing at 0x2000 + offset
	
	// for (c = 0; c < NUM_CHARS; c++)
	//   for (r = 0; r < 8; r++) // 8 bytes per character
	//     offset++
	//     split address up into two bytes
	//     put most sig. byte into R18 : addr / 256
	//     put least sig. byte into R19 : addr % 256
	//     put data to be written into R31
	//   next r
	// next c
	

}