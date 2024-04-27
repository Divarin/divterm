#define VID_VIC 0
#define VID_VDC 1
#define VID_SPLIT 2
#define BUFFER_SIZE 20000

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

void setBaud(int baud);
void setVideo(int video);
void term();