#define VERSION	"Version 0.5 250104\n"
#define VID_VIC 0
#define VID_VDC 1
#define VID_SPLIT 2
#define BUFFER_SIZE 4000
#define ANSI_BUFFER_SIZE 20
#define SCROLLBACK_SIZE 2000
#define BUFFER_END (bs+4000)
#define SCROLL_AMT  256
#define CH_DOWN		17
#define CH_UP		145
#define CH_LEFT		157
#define CH_RIGHT	29
#define CH_CURSOR	164
#define CH_HOME		19
#define CH_CLR		147
#define CH_QUOTE	34
#define CH_BACKSPACE 20

// black, white, red, cyan, magenta, green, blue, yellow
// orange, brown, light red, dark grey, medium grey, light green, 
// light blue, light grey 

#define CH_BLACK 		144
#define CH_WHITE		5
#define CH_RED			28
#define CH_LIGHT_RED 	150
#define CH_CYAN			159
#define CH_MAGENTA		156
#define CH_GREEN		30
#define CH_LIGHT_GREEN 	153
#define CH_BLUE			31
#define CH_LIGHT_BLUE	154
#define CH_ORANGE		129
#define CH_YELLOW		158
#define CH_GRAY			151
#define CH_LIGHT_GRAY	155
#define CH_REV_ON	18
#define CH_REV_OFF	146
#define CH_SWITCH_UP 142
#define CH_SWITCH_DN 14
#define CH_LOCAL_ECHO 177

#define ClearCursor putchar(' '); putchar(157)
#define NUM_EMUS 2
#define EMU_CBM 0
#define EMU_ASCII 1

#define bool int
#define false 0
#define true 1
#define SW_PAUSE 1
#define SW_COLLECTING_ANSI 2
#define SW_DECODE_ANSI 4
#define SW_FAST_VDC 8
#define SW_CURSOR 16
#define SW_DEBUG 32
#define SW_PRINT_CR 64
#define SW_CURSOR_OVERRIDE 128
#define SW_LOCAL_ECHO 256

#define SCREENTOP	0x0400
#define COLORTOP	0xd800
#define CHARDEFS 0x1000
#define CHARDEFS_PTR 0x1000/1024

#define FILE_EXTENDED_ASCII "extascii"
#define FILE_PETSCII "petscii"

struct ser_params p300 = {
	SER_BAUD_300,
	SER_BITS_8,
	SER_STOP_1,
	SER_PAR_NONE,
	SER_HS_HW
};
struct ser_params p1200 = {
	SER_BAUD_1200,
	SER_BITS_8,
	SER_STOP_1,
	SER_PAR_NONE,
	SER_HS_HW
};
struct ser_params p2400 = {
	SER_BAUD_2400,
	SER_BITS_8,
	SER_STOP_1,
	SER_PAR_NONE,
	SER_HS_HW
};
struct ser_params p4800 = {
	SER_BAUD_4800,
	SER_BITS_8,
	SER_STOP_1,
	SER_PAR_NONE,
	SER_HS_HW
};
struct ser_params p9600 = {
	SER_BAUD_9600,
	SER_BITS_8,
	SER_STOP_1,
	SER_PAR_NONE,
	SER_HS_HW
};
struct ser_params p19200 = {
	SER_BAUD_19200,
	SER_BITS_8,
	SER_STOP_1,
	SER_PAR_NONE,
	SER_HS_HW
};
struct ser_params p38400 = {
	SER_BAUD_38400,
	SER_BITS_8,
	SER_STOP_1,
	SER_PAR_NONE,
	SER_HS_HW
};

void loadAsciiMap();
void setBaud(int baud);
void setVideo(int video, bool fast);
void setEmu(int emu);
void term();
void showHelp();
void parseAnsi();
void parseAnsiColor();
void parseAnsiHome();
void parseAnsiCursor(char direction);
void loadFont(const char* filename);
char* showBool(bool expression);