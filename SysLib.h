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
	
	SVECTOR VScale={0};

	ResetCallback();

	// Initialize the GS
	
	SetVideoMode(MODE_PAL);
	ResetGraph(0);
	
	GsInitGraph(SCREEN_XRES, SCREEN_YRES, GsOFSGPU, DITHER, 0);	
	GsDefDispBuff(0, 0, 0, 0);
	
	// Prepare the ordering tables

	myOT[0].length	=OT_LENGTH;
	myOT[1].length	=OT_LENGTH;
	myOT[0].org		=myOT_TAG[0];
	myOT[1].org		=myOT_TAG[1];
	
	GsClearOt(0, 0, &myOT[0]);
	GsClearOt(0, 0, &myOT[1]);
	
	// Setup 3D and projection matrix

	GsInit3D();
	GsSetProjection(CENTERX);
	
	// Initialize coordinates for the camera (it will be used as a base for future matrix calculations)

	GsInitCoordinate2(WORLD, &Camera.coord2);

	// Set the ambient color (for lighting)

	GsSetAmbient(ONE/4, ONE/4, ONE/4);
	
	// Set the default lighting mode

	GsSetLightMode(0);
	
	// Set the light source coordinates

	pslt.vx = 0;
	pslt.vy = 0;
	pslt.vz = 1000;
	
	pslt.r = 0xff; pslt.g = 0xff; pslt.b = 0xff;
	
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
	// Reset font position

	Font_ResetPos();

	// Get active buffer ID and clear the OT to be processed for the next frame

	myActiveBuff = GsGetActiveBuff();
	GsSetWorkBase((PACKET*)myPacketArea[myActiveBuff]);
	GsClearOt(0, 0, &myOT[myActiveBuff]);
}

void Display() {
	// Switch buffers, draw the old frame

	GsSwapDispBuff();
	GsSortClear(0, 0, 0, &myOT[myActiveBuff]);
	GsDrawOt(&myOT[myActiveBuff]);	
}