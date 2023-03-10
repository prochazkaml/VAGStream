#include "SysLib.h"
#include "Assets.h"

#define PACKETMAX	2048
#define GPU_GP1 (*((volatile unsigned long *) (0xBF801814))) // thanks, PSn00bSDK

GsOT OT[2]; // OT handlers
GsOT_TAG OT_TAG[2][OT_ENTRIES]; // OT tables
PACKET PacketArea[2][PACKETMAX * 24]; // Packet buffers
int ActiveBuff = 0; // Active buffer

unsigned long __ramsize =   0x002000000; // force 2 megabytes of RAM
unsigned long __stacksize = 0x00004000; // force 16 kilobytes of stack

unsigned long gpu_status = 0;

void LoadTexture(u_long *addr) {
	// A simple TIM loader... Not much to explain
	
	RECT rect;
	GsIMAGE tim;
	
	// Get TIM information
	GsGetTimInfo((addr+1), &tim);
	
	// Load the texture image
	rect.x = tim.px;	rect.y = tim.py;
	rect.w = tim.pw;	rect.h = tim.ph;
	LoadImage(&rect, tim.pixel);
	DrawSync(0);
	
	// Load the CLUT (if present)
	if ((tim.pmode>>3) & 0x01) {
		rect.x = tim.cx;	rect.y = tim.cy;
		rect.w = tim.cw;	rect.h = tim.ch;
		LoadImage(&rect, tim.clut);
		DrawSync(0);
	}
}

void init() {
	SpuCommonAttr c_attr;
	SpuVoiceAttr s_attr;
	
	ResetCallback();

	// Initialize the GS
	
	SetVideoMode(MODE_PAL);
	ResetGraph(0);
	
	GsInitGraph(SCREEN_XRES, SCREEN_YRES, GsOFSGPU, DITHER, 0);	
	GsDefDispBuff(0, 0, 0, 0);
	
	// Prepare the ordering tables

	OT[0].length = OT_LENGTH;
	OT[1].length = OT_LENGTH;
	OT[0].org = OT_TAG[0];
	OT[1].org = OT_TAG[1];
	
	GsClearOt(0, 0, &OT[0]);
	GsClearOt(0, 0, &OT[1]);
	
	// Setup 3D and projection matrix

	Init_3DLib();

	// Load the textures

	LoadTexture((u_long*)assets_FONT_TIM);

	// Initialize the controller
	
	PadInit(0);

	// Init SPU

	SpuInit();

	c_attr.mask = (SPU_COMMON_MVOLL | SPU_COMMON_MVOLR);
	c_attr.mvol.left  = MAX_VOLUME;
	c_attr.mvol.right = MAX_VOLUME;

	SpuSetCommonAttr(&c_attr);
	SpuSetTransferMode(SpuTransByDMA);

	s_attr.mask = (	SPU_VOICE_VOLL | SPU_VOICE_VOLR | SPU_VOICE_PITCH |
					SPU_VOICE_ADSR_AMODE | SPU_VOICE_ADSR_SMODE | SPU_VOICE_ADSR_RMODE |
					SPU_VOICE_ADSR_AR | SPU_VOICE_ADSR_DR | SPU_VOICE_ADSR_SR | SPU_VOICE_ADSR_RR | SPU_VOICE_ADSR_SL);

	s_attr.voice		= SPU_ALLCH;
	s_attr.volume.left  = MAX_VOLUME;
	s_attr.volume.right = MAX_VOLUME;
	s_attr.a_mode       = SPU_VOICE_LINEARIncN;
	s_attr.s_mode       = SPU_VOICE_LINEARIncN;
	s_attr.r_mode       = SPU_VOICE_LINEARDecN;
	s_attr.ar           = 0x0;//0x20;
	s_attr.dr           = 0x0;
	s_attr.sr           = 0x0;
	s_attr.rr           = 0x0;
	s_attr.sl           = 0x0;
	s_attr.pitch        = 0x1000;

	SpuSetVoiceAttr(&s_attr);
	
	SpuSetKey(SPU_OFF, SPU_ALLCH);

	CdInit();
}

void PrepDisplay() {
	// Wait for the GPU to start rendering a new field

	while (!((GPU_GP1 ^ gpu_status) & (1 << 31)));

	gpu_status = GPU_GP1;

	// Reset font position

	Font_ResetPos();

	// Get active buffer ID and clear the OT to be processed for the next frame

	ActiveBuff = GsGetActiveBuff();
	GsSetWorkBase((PACKET*)PacketArea[ActiveBuff]);
	GsClearOt(0, 0, &OT[ActiveBuff]);
}

void Display() {
	// Switch buffers, draw the old frame

	GsSwapDispBuff();
	GsSortClear(0, 0, 0, &OT[ActiveBuff]);
	GsDrawOt(&OT[ActiveBuff]);	
}
