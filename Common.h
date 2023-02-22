#define SCREEN_XRES		640
#define SCREEN_YRES 	480
#define DITHER			1

#define CENTERX			SCREEN_XRES/2
#define CENTERY			SCREEN_YRES/2

// Increasing this value (max is 14) reduces sorting errors in certain cases
#define OT_LENGTH	12

#define OT_ENTRIES	1<<OT_LENGTH
#define PACKETMAX	2048

GsOT OT[2]; // OT handlers
GsOT_TAG OT_TAG[2][OT_ENTRIES]; // OT tables
PACKET PacketArea[2][PACKETMAX * 24]; // Packet buffers
int ActiveBuff = 0; // Active buffer

unsigned long __ramsize =   0x002000000; // force 2 megabytes of RAM
unsigned long __stacksize = 0x00004000; // force 16 kilobytes of stack

#define	MAX_VOLUME	0x3FFF
