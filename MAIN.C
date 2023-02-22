#include <sys/types.h>
#include <KERNEL.h>
#include <LIBETC.H>
#include <LIBGTE.H>
#include <LIBGPU.H>
#include <LIBGS.H>
#include <LIBSPU.H>
#include <LIBCD.H>
#include <LIBSND.H>
#include <LIBSN.H>
#include <RAND.H>
#include <stdio.h>

#include "Assets.h"

#include "Common.h"
#include "FontLib.h"
#include "SysLib.h"
#include "3DLib.h"
#include "ThreadLib.h"
#include "StreamLib.h"
#include "InputLib.h"

// TODO: Handle drive errors, buffer underruns, open drive trays!

POLY_F4 buffermeter;
int control = 0;
	
void debug_screen() {
	char buffer[64];
	int i, padx;

	PrepDisplay();

	Font_ChangeColor(255, 255, 255);

	Font_ChangePosition(0, -240);
	Font_PrintStringCentered("VAG Stream test");

	sprintf(buffer, "%d total sectors read", sectors_read);
	Font_ChangePosition(0, -240 + 64);
	Font_PrintStringCentered(buffer);

	sprintf(buffer, "Latest loaded chunk ID: %d", last_sector_id);
	Font_ChangePosition(0, -240 + 96);
	Font_PrintStringCentered(buffer);

	sprintf(buffer, "%d data sectors to be loaded", remaining_data_sectors);
	Font_ChangePosition(0, -240 + 128);
	Font_PrintStringCentered(buffer);

	sprintf(buffer, "%d audio sectors to be loaded", remaining_audio_sectors);
	Font_ChangePosition(0, -240 + 160);
	Font_PrintStringCentered(buffer);

	sprintf(buffer, "SPU IRQ addr 0x%X", SpuGetIRQAddr());
	Font_ChangePosition(0, -240 + 192);
	Font_PrintStringCentered(buffer);

	sprintf(buffer, "%d target chunk", target_chunk);
	Font_ChangePosition(-160, -240 + 224);
	Font_PrintStringCentered(buffer);

	sprintf(buffer, "%d chunks read", current_chunk);
	Font_ChangePosition(160, -240 + 224);
	Font_PrintStringCentered(buffer);

	sprintf(buffer, "%d chunks processed", transferred_chunks);
	Font_ChangePosition(0, -240 + 256);
	Font_PrintStringCentered(buffer);

	sprintf(buffer, "%d chunks processed", transferred_chunks);
	Font_ChangePosition(0, -240 + 256);
	Font_PrintStringCentered(buffer);

	i = 320 * ((current_chunk - transferred_chunks) * 64 - remaining_audio_sectors) / 256;

	setXY4(&buffermeter, 0, 240 - 160, i, 240 - 160, 0, 240 - 128, i, 240 - 128);
	GsSortPoly(&buffermeter, &OT[ActiveBuff], OT_ENTRIES-1);

	sprintf(buffer, "Buffer %d %% full", i * 100 / 320);
	Font_ChangePosition(-320, 240 - 160);
	Font_PrintString(buffer);

	sprintf(buffer, "Drive status 0x%X", CdStatus());
	Font_ChangePosition(0, 240 - 96);
	Font_PrintStringCentered(buffer);

	sprintf(buffer, "Initial checksum %d", checksum);
	Font_ChangePosition(0, 240 - 64);
	Font_PrintStringCentered(buffer);

	sprintf(buffer, "%d %d I'm not dead", control, VSync(-1));
	Font_ChangePosition(0, 240 - 32);
	Font_PrintStringCentered(buffer);

	Display();
}

void error_screen() {
	char buffer[64];

	PrepDisplay();

	Font_ChangeColor(255, 255, 255);

	Font_ChangePosition(0, -16);
	Font_PrintStringCentered("Error loading file!");

	sprintf(buffer, "%d I'm not dead", VSync(-1));
	Font_ChangePosition(0, 240 - 32);
	Font_PrintStringCentered(buffer);

	Display();
}

void subthread() {
	if(StartStream("\\TEST.PAK;1")) {
		control = -1;
		while(control != 1);

		control = 2;
		while(1);
	}

	while(control != 1) {
		if(ProcessStream()) break;
	}
		
	StopStream();

	control = 2;
	while(1);
}

int main() {
	int padx;

	// Initialize system

	init();

	SetPolyF4(&buffermeter);
	setRGB0(&buffermeter, 255, 255, 255);

	while(1) {
		InitSubThread(subthread);

		VSyncCallback(ReturnToMainThread);

		control = 0;

		while(control < 2) {
			if(control >= 0)
				debug_screen();
			else
				error_screen();

			padx = ParsePad(0, 0);

			if(padx & PADRdown) control = 1;

			RunSubThread();
		}

		VSyncCallback(NULL);

		StopSubThread();
	}
}