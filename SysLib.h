#include <SYS/TYPES.H>
#include <LIBGTE.H>
#include <LIBGPU.H>
#include <LIBGS.H>
#include <LIBSPU.H>
#include <LIBETC.H>

void LoadTexture(u_long *addr);
void init();
void PrepDisplay();
void Display();

#define SCREEN_XRES		640
#define SCREEN_YRES 	480
#define DITHER			1

#define CENTERX			SCREEN_XRES/2
#define CENTERY			SCREEN_YRES/2

// Increasing this value (max is 14) reduces sorting errors in certain cases
#define OT_LENGTH	12
#define OT_ENTRIES	1<<OT_LENGTH

extern GsOT OT[2]; // OT handlers
extern int ActiveBuff; // Active buffer

#define	MAX_VOLUME	0x3FFF
