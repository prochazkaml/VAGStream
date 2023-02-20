#include <sys/types.h>
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
#include "StreamLib.h"
#include "InputLib.h"

// TODO: Handle drive errors, buffer underruns, open drive trays!

POLY_F4 buffermeter;
	
void vsync_callback() {
	char buffer[64];
	int i;
	static int x = 0;

	Display();

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
	GsSortPoly(&buffermeter, &myOT[myActiveBuff], OT_ENTRIES-1);

	sprintf(buffer, "Buffer %d %% full", i * 100 / 320);
	Font_ChangePosition(-320, 240 - 160);
	Font_PrintString(buffer);

	sprintf(buffer, "Drive status 0x%X", CdStatus());
	Font_ChangePosition(0, 240 - 96);
	Font_PrintStringCentered(buffer);

	sprintf(buffer, "Initial checksum %d", checksum);
	Font_ChangePosition(0, 240 - 64);
	Font_PrintStringCentered(buffer);

	sprintf(buffer, "%d %d I'm not dead", spu_transfer_progress, x++);
	Font_ChangePosition(0, 240 - 32);
	Font_PrintStringCentered(buffer);
}

int main() {
	int padx;

	// Initialize system

	init();

	// Start the main display thread

	SetPolyF4(&buffermeter);
	setRGB0(&buffermeter, 255, 255, 255);

	VSyncCallback(vsync_callback);

	// Run the stream loop forever

	while(1) {
		StartStream("\\TEST.PAK;1");

		while(1) {
			if(ProcessStream()) break;

			padx = ParsePad(0, 0);

			if(padx & PADRdown) break;
		}
		
		StopStream();
	}
}