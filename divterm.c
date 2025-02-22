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
#include <ctype.h>
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
char asciiMapIn[256];
char asciiMapOut[256];
char careful[256]; // buffer for careful-send
int flags;

int main(void)
{	
	driveNum = PEEK(4096);
	
	ansiBufferIndex = 0;
	currentBaud = 4;
	currentVideo = 0;
	currentEmu = EMU_CBM;
	isReverse = false;
	flags = SW_FAST_VDC | SW_DECODE_ANSI | SW_CURSOR | SW_CURSOR_OVERRIDE;

	loadAsciiMap();
	
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

	POKE(0x026a, 0); // make cursor visible

	term();

	// since term() contains an infinite loop we never should get here
	
	printf("Closing Terminal\n\n"); // but it'd be nice to know if we did
	cgetc();
	
	ser_close();
	exit(0);
	
	return EXIT_SUCCESS;
}

void loadAsciiMap()
{
	int i;
	
	// asciiMapIn
	// converts incoming bytes to the byte we'll print
	// (when in ASCII mode)
	for (i=0; i < 256; i++)
	{
		// uppercase letters
		if (i >= 65 && i <= 90)
		{
			asciiMapIn[i] = i+128;
			continue;
		}
		
		// lowercase letters
		if (i >= 97 && i <= 122)
		{
			asciiMapIn[i] = 65+(i-97);
			continue;
		}
		
		if (i == 8)
		{
			asciiMapIn[i] = 20;
			continue;
		}
		
		switch (i) {
			case CH_WHITE: // 5
			case CH_SWITCH_DN: // 14
			case CH_DOWN: // 17
			case CH_REV_ON: // 18
			case CH_HOME: // 19
			case CH_RED: // 28
			case CH_RIGHT: // 29
			case CH_GREEN: // 30
				asciiMapIn[i] = 0;
				continue;
			case 92: asciiMapIn[i] =  223; continue; // backslash (custom)
			case 95: asciiMapIn[i] =  228; continue; // underscore
			case 128: asciiMapIn[i] =  'C'; continue;
			case 129: case 150: case 151: case 163: case 230: asciiMapIn[i] =  'u'; continue;
			case 130: case 136: case 137: case 138: asciiMapIn[i] =  'e'; continue;
			case 131: case 132: case 133: case 134: case 160: case 166: case 224: asciiMapIn[i] =  'a'; continue;
			case 135: case 155: asciiMapIn[i] =  'c'; continue;
			case 139: case 140: case 141: case 161: case 173: asciiMapIn[i] =  'i'; continue;
			case 142: case 143: asciiMapIn[i] =  'A'; continue;
			case 144: asciiMapIn[i] =  'E'; continue;
			case 145: case 146: case 158: case 159: case 171: case 172: case 226: case 231:
			case 232: case 234: case 235: case 236: case 237: case 238: case 239: case 241:
			case 242: case 243: case 244: case 245: case 251: case 253:
				asciiMapIn[i] = '?'; // not mapped to anything useful
				continue;
			case 147: case 148: case 149: case 162: case 248: asciiMapIn[i] =  'o'; continue;
			case 152: asciiMapIn[i] =  'y'; continue;
			case 153: case 233: case 229: asciiMapIn[i] =  'O'; continue;
			case 154: asciiMapIn[i] =  'U'; continue;
			case 157: asciiMapIn[i] =  168; continue; // yen (custom)
			case 164: case 252: asciiMapIn[i] =  'n'; continue;
			case 165: asciiMapIn[i] =  'N'; continue;
			case 225: asciiMapIn[i] =  'B'; continue;
			case 156: asciiMapIn[i] =  92; continue; // british pound
			case 174: asciiMapIn[i] = '<'; continue;
			case 175: asciiMapIn[i] = '>'; continue;
			case 176: asciiMapIn[i] =  165; continue; // thin hash (custom)
			case 177: asciiMapIn[i] =  166; continue; // medium hash (custom)
			case 178: asciiMapIn[i] =  167; continue; // thick hash (custom)
			case 179: case 186: asciiMapIn[i] =  163; continue; // center vertical line (custom)
			case 180: case 181: case 182: case 185: asciiMapIn[i] =  179; continue; // ┤
			case 191: case 183: case 184: case 187: case 170: asciiMapIn[i] =  174; continue; // ┐
			case 192: case 200: case 211: case 212: asciiMapIn[i] =  173; continue; // └
			case 193: case 202: case 207: case 208: asciiMapIn[i] =  177; continue; // ┴
			case 194: case 203: case 209: case 210: asciiMapIn[i] =  178; continue; // ┬
			case 195: case 198: case 199: case 204: asciiMapIn[i] =  171; continue; // ├
			case 196: case 205: asciiMapIn[i] =  96; continue; // center horizontal line
			case 197: case 206: case 215: case 216: asciiMapIn[i] =  219; continue; // ┼
			case 217: case 188: case 189: case 190: asciiMapIn[i] =  189; continue; // ┘
			case 218: case 201: case 213: case 214: case 169: asciiMapIn[i] =  176; continue; // ┌
			case 219: asciiMapIn[i] =  220; continue; // solid block (custom)
			case 220: asciiMapIn[i] =  162; continue; // lower block
			case 221: asciiMapIn[i] =  161; continue; // left block
			case 222: asciiMapIn[i] =  182; continue; // right block
			case 223: asciiMapIn[i] =  184; continue; // upper block
			case 227: asciiMapIn[i] =  126; continue; // pi (custom)
			case 240: case 247: asciiMapIn[i] = '='; continue;
			case 246: asciiMapIn[i] = '/'; continue;
			case 249: case 250: asciiMapIn[i] =  '.'; continue;
			case 254: asciiMapIn[i] =  190; continue;// ▘
			case 255: asciiMapIn[i] = 0; continue; // don't show 255 characters
		}
		
		// default
		asciiMapIn[i] = i;
	}
	
	// asciiMapOut
	// converts bytes the user typed to what we're sending
	// (when in ASCII mode)
	for (i=0; i < 256; i++)
	{
		// uppercase letters
		if (i >= 193 && i <= 218)
		{
			asciiMapOut[i] = 65 + (i-193);
			continue;
		}

		// lowercase letters
		if (i >= 65 && i <= 90)
		{
			asciiMapOut[i] = 97 + (i-65);
			continue;
		}

		// default
		asciiMapOut[i] = i;
	}
	
	asciiMapOut[20] = 8; // backspace
}

void term()
{
	int i;
	char *bs; // buffer start
	char *rp; // buffer read pointer
	char *wp; // buffer write pointer
	char *tmp;
	int scrollToEnd;
	// since scroll-up falls through to scroll-down this flag is used to indicate that we're scrolling up
	// this is used because while in scroll-down there's some logic that only happens if we're scrolling up
	bool scrollUp; 

	i = 0;
	
	ansiBufferIndex = 0;
	
	bs = (char*)(malloc(BUFFER_SIZE));
	
	wp = bs; // initialize write pointer to start of buffer
	
	printf(" -- DivTerm Ready --\n");
	printf(VERSION);
	
	showHelp();
	
	// main loop
	while (true)
    {
        char chr;

        if (kbhit())
        {
            chr = cgetc();
			if (flags & SW_PAUSE)
			{
				scrollToEnd = SCROLL_AMT;
				scrollUp = false;
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
						scrollUp = true;
						// fall through to scroll forward
					case CH_DOWN:
						// scroll-forward
						for (i=0; i < scrollToEnd && rp != wp; i++)
						{
							rp++;
							if (rp >= BUFFER_END) rp = bs;
							if (rp != wp && *rp != CH_CLR && *rp != CH_HOME) putchar(*rp);
							// if scrolling up and cursor has reached bottom of screen then stop scrolling
							if (scrollUp && currentVideo == VID_VIC && wherey() >= 24)
								break;
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
						ClearCursor;
						flags &= ~SW_CURSOR;
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
					printf("\nDecode ANSI: %s\n", showBool(flags & SW_DECODE_ANSI));
					continue;
				case CH_F3:
					ClearCursor;
					setBaud((currentBaud + 1) % 7);
					continue;
				case CH_F4:
					if (flags & SW_DEBUG)
						flags &= ~SW_DEBUG;
					else
						flags |= SW_DEBUG;
					printf("\nDebug: %s\n", showBool(flags & SW_DEBUG));
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
					printf("\nFast VDC mode: %s\n", showBool(flags & SW_FAST_VDC));
					continue;
				case CH_F7:
					ClearCursor;
					setEmu((currentEmu + 1) % NUM_EMUS);
					continue;
				case CH_F8:
					if (flags & SW_CURSOR_OVERRIDE)
						flags &= ~SW_CURSOR_OVERRIDE;
					else
						flags |= SW_CURSOR_OVERRIDE;
					ClearCursor;
					continue;
				case CH_LOCAL_ECHO:
					if (flags & SW_LOCAL_ECHO)
						flags &= ~SW_LOCAL_ECHO;
					else
						flags |= SW_LOCAL_ECHO;
					ClearCursor;
					printf("\nLocal Echo: %s\n", showBool(flags & SW_LOCAL_ECHO));						
					continue;
				// case CH_CAREFUL_SEND:
				// 	ClearCursor;
				// 	carefulSend();
				// 	continue;
			}
			
			if (currentEmu == EMU_ASCII)
			{
				chr = asciiMapOut[chr];
				if (chr == 13)
					flags |= SW_PRINT_CR;			
			}
			
			ser_put(chr);
			if (flags & SW_LOCAL_ECHO)
				putchar(chr);
        }

        while (ser_get (&chr) == SER_ERR_OK)
		{
			if (flags & SW_DEBUG)
			{
				if (currentVideo == VID_VIC)
				{
					videomode(5);
					printf("%c:%i ", chr, chr);
					videomode(1);
				}
				else
				{
					videomode(1);
					printf("%c:%i ", chr, chr);
					videomode(5);
				}
				if (kbhit() && cgetc() == CH_F4)
				{
					flags &= ~SW_DEBUG;
				}
			}

			if (currentEmu == EMU_CBM && (chr == 9 || chr == 10)) continue;
			if (currentEmu == EMU_ASCII)
			{
				// in ASCII mode generally we don't print CR but instead interpret LFs as CR
				// however, if the user typed a CR AND we received a CR in echo then we'll print that.
				// SW_PRINT_CR is switched on when enter is pressed, and then turned off immedately after use.
				if (chr == 13)
				{
					if (flags & SW_PRINT_CR)
					{
						flags &= ~SW_PRINT_CR;
					}
					else
						continue;
				}
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
			if (currentEmu == EMU_ASCII)
			{
				chr = asciiMapIn[chr];
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
			if (flags & SW_CURSOR && flags & SW_CURSOR_OVERRIDE)
			{
				switch (chr)
				{
					case 13:
					case 10:
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
					case CH_BACKSPACE:
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
		
		if (flags & SW_CURSOR_OVERRIDE)
		{
			flags |= SW_CURSOR;
			putchar(CH_CURSOR);
			putchar(CH_LEFT);
		}
    }
}

void carefulSend()
{
	int i;
	int retryCount;
	char c;
	char echo;

	printf("careful-send: ");
	i = 0;
	do
	{
		while (!kbhit())
        {
			// wait for keyboard to be hit
		}
		c = cgetc();
		if (c == '\n' || i >= 255)
			break;
		else if (c == CH_BACKSPACE)
		{
			if (i > 0)
			{
				putchar(c);
				i--;
			}
			// otherwise do nothing (string is empty, nothing to delete)
		}
		else
		{
			putchar(c);
			careful[i++] = c;
		}
	} while (1);
	
	if (i == 0)
		return; // didn't type anything

	careful[i] = 0; // terminate string
	printf("\n");
	
	for (i = 0; i < 256; i++)
	{
		c = careful[i];
		if (!c)
			break;
		retryCount = 0;
		do
		{
			ser_put(c);
			echo = 0;
			while (ser_get(&echo) != SER_ERR_OK)
			{
				// keep waiting for echo
			}
			if (currentEmu == EMU_ASCII)
				echo = asciiMapIn[echo];
			printf("\nsent: %s(%i) got: %s(%i)\n", c, c, echo, echo);
			if (!echo)
				return;
			putchar(echo);

			//printf("sent: %s(%i) got: %s(%i)\n", c, c, echo, echo);
			if (toupper(echo) == toupper(c))
				break;
			if (currentEmu == EMU_CBM && echo == c - 96)
				break;
			printf("\nsent: %s(%i) got: %s(%i)\n", c, c, echo, echo);
			if (++retryCount > 5)
				return;
			ser_put(CH_BACKSPACE);
			//putchar(CH_BACKSPACE);
			printf("sending backspace to retry\n");
		} while (1);
	}
}

char* showBool(bool expression)
{
	if (expression)
		return "True";
	return "False";
}

void setVideo(int video, bool fast)
{
	currentVideo=video;
	
	switch (currentVideo)
	{
		case 1:
			// 80 col VDC
			printf("\n\nSetting Video Mode: 80 Column VDC\n");
			if (fast) set_c128_speed(1);
			videomode(5);
			printf("\n\nSetting Video Mode: 80 Column VDC\n");
			break;
		case 2:
			// 40 & 80 together
			printf("\n\nSetting Video Mode: VIC & VDC\n");
			break;
		default:
			// 40 col VIC
			printf("\n\nSetting Video Mode: 40 Column VIC\n");
			set_c128_speed(0);
			videomode(1);
			printf("\n\nSetting Video Mode: 40 Column VIC\n");			
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
			loadFont(FILE_EXTENDED_ASCII);
			break;
		default:
			currentEmu = EMU_CBM;
			printf("\n\nPETSCII/CBM emulation\n\n");
			loadFont(FILE_PETSCII);
			break;
	}
}

void showHelp() {	
	printf("\n -- DivTerm --\n");
	//      1234567890123456789012345678901234567890
	printf("F1: This Menu  F2: Decode Ansi   : %s\n", showBool(flags & SW_DECODE_ANSI));
	printf("F3: Set Baud   F4: Debug Mode    : %s\n", showBool(flags & SW_DEBUG));
	printf("F5: 40/80 Col  F6: Fast VDC      : %s\n", showBool(flags & SW_FAST_VDC));
	printf("F7: Emulation  F8: Toggle Cursor : %s\n", showBool(flags & SW_CURSOR_OVERRIDE));
	printf("C= + E: Toggle Local Echo\n");
	//printf("C= + C: Careful Send\n");
	printf("Free RAM: %u\n\n", _heapmemavail());
}

void parseAnsi()
{	
	switch (ansiBuffer[ansiBufferIndex-1])
	{
		case 109: // 'm'
		case 77: // 'M'
			parseAnsiColor();
			break;
		case 106: // 'j'
		case 74: // 'J'
			// clear screen
			if (ansiBuffer[ansiBufferIndex-2] == '2') {
				putchar(CH_CLR);
			}
			break;
		case 70: // 'f'
		case 104: // 'h'
		case 72: // 'H'
			parseAnsiHome();
			break;
		case 97: // 'a'
		case 65: // 'A'
			parseAnsiCursor(CH_UP);
			break;
		case 98: // 'b'
		case 66: // 'B'
			parseAnsiCursor(CH_DOWN);
			break;
		case 99: // 'c'
		case 67: // 'C'
			parseAnsiCursor(CH_RIGHT);
			break;
		case 100: // 'd'
		case 68: // 'D'
			parseAnsiCursor(CH_LEFT);
			break;
	}
}
			
void parseAnsiColor()
{
	char* code;
	char* num;
	int n;
	bool isBright;
	char colorCompliment;

	// when setting a color it's either bright or dim
	// if setting a bright color, then set colorCompliment to the dim version
	// if setting a dim color, then set the colorCompliment to the bright version
	// the colorCompliment might be used if the bright flag (1) or dim flag (2)
	// is found *after* the color flag.
	colorCompliment = 0;
	isBright = false;
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
				if (isReverse)
				{
					putchar(CH_REV_OFF);
					isReverse = false;
					putchar(CH_WHITE);
				}
				break;
			case 1:
				isBright = true;
				if (colorCompliment) putchar(colorCompliment);
				break;
			case 2:
				isBright = false;
				if (colorCompliment) putchar(colorCompliment);
				break;
			case 7:
				// explicit reverse
				putchar(CH_REV_ON);
				isReverse = true;
				break;
			case 30: // black
				if (isBright)
				{
					putchar(CH_GRAY);
					colorCompliment = CH_BLACK;
				}
				else
				{
					putchar(CH_BLACK); 
					colorCompliment = CH_GRAY;
				}
				break;
			case 40: break; // black background, handled in IF above, don't touch foreground color
			case 31: case 41:
				if (isBright)
				{
					putchar(CH_LIGHT_RED);
					colorCompliment = CH_RED;
				}
				else
				{
					putchar(CH_RED);
					colorCompliment = CH_LIGHT_RED;
				}
				break;
			case 32: case 42:
				if (isBright)
				{
					putchar(CH_LIGHT_GREEN);
					colorCompliment = CH_GREEN;
				}
				else
				{
					putchar(CH_GREEN);
					colorCompliment = CH_LIGHT_GREEN;
				}
				break;
			case 33: case 43:
				if (isBright)
				{
					putchar(CH_YELLOW);
					colorCompliment = CH_ORANGE;
				}
				else
				{
					putchar(CH_ORANGE);
					colorCompliment = CH_YELLOW;
				}
				break;
			case 34: case 44:
				if (isBright)
				{
					putchar(CH_LIGHT_BLUE);
					colorCompliment = CH_BLUE;
				}
				else
				{
					putchar(CH_BLUE);
					colorCompliment = CH_LIGHT_BLUE;
				}
				break;
			case 35: case 45:
				putchar(CH_MAGENTA); // there is no dark magenta
				break;
			case 36: case 46:
				putchar(CH_CYAN); // there is no dark cyan
				break;
			case 37: case 47:
				if (isBright)
				{
					putchar(CH_WHITE);
					colorCompliment = CH_LIGHT_GRAY;
				}
				else
				{
					putchar(CH_LIGHT_GRAY);
					colorCompliment = CH_WHITE;
				}
				break;
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
	if (!strlen(num))
		n = 1;
	else
		n = atoi(num);
	
	if (n) ClearCursor;
		
	for (i=0; i < n; i++) {
		putchar(direction);
	}
}

// loads custom characters for ascii & extended ascii characters
// this includes any character that can't be direclty mapped to a petscii character
void loadFont(const char* filename)
{
	int bytesRead;
	int r;
	int n;
	int i;
	int offset;
	unsigned char buffer[9];
	
	printf("\nhang on loading font...");
	cbm_open(2, driveNum, 0, filename);
	
	do
	{
		bytesRead = cbm_read(2, buffer, 9);
		if (bytesRead == 0)
			break;
		if (bytesRead < 9)
		{
			printf("\nerror in font file %s, wrong number of bytes!\n", filename);
			break;
		}
		n = buffer[0]; // offset to address where the pattern lives
		
		for (i = n; i <= n+128; i+=128)
		{
			offset=0x1000 + 16*i;
			for (r=1; r < bytesRead; r++) {
				// set address of pattern.  This is two bytes.
				// poke the first byte into address register (most sig. byte into R18)
				POKE(0xd600, 18);
				while (!(PEEK(0xd600) & 128)) { }
				POKE(0xd601, (0x2000 + offset)/256);

				// poke the second byte into address register (least sig. byte into R19)
				POKE(0xd600, 19);
				while (!(PEEK(0xd600) & 128)) { }
				POKE(0xd601, (0x2000 + offset)%256);

				// poke the byte of the pattern into the data register (R31)
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
		
	cbm_close(2);
	printf(" done!\n\n");
}