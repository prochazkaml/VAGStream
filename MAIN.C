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
#include "CDLib.h"
#include "InputLib.h"

// TODO: Handle drive errors, buffer underruns, open drive trays!

SpuIRQCallbackProc spu_callback() {
	SpuSetIRQ(SPU_OFF);
}

int playback_status; // 0 = not started, 1 = running, 2 = ended

int spu_transfer_progress;
int transferred_chunks;

int checksum;

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

void run_test() {
	int i, x = 0, y;

	int padx;
	char buffer[64];

	playback_status = 0;
	transferred_chunks = 0;
	spu_transfer_progress = 0;
	checksum = 0;

	// Main program loop

	PlayCD();

	SetPolyF4(&buffermeter);
	setRGB0(&buffermeter, 255, 255, 255);

	VSyncCallback(vsync_callback);

	do {
		if(!callback_running && !playback_status) {
			for(i = 0; i < 131072 * 4; i++) {
				checksum += audiobuffer[i];
			}

			SpuSetKey(SPU_OFF, SPU_ALLCH);

			SpuSetTransferStartAddr(0x1010);
			SpuWrite(audiobuffer, 65536);
			SpuIsTransferCompleted(SPU_TRANSFER_WAIT);

			SpuSetTransferStartAddr(0x11010);
			SpuWrite(audiobuffer + 65536 * 2, 65536);
			SpuIsTransferCompleted(SPU_TRANSFER_WAIT);

			SpuSetTransferStartAddr(0x21010);
			SpuWrite(audiobuffer + 65536 * 1, 65536);
			SpuIsTransferCompleted(SPU_TRANSFER_WAIT);

			SpuSetTransferStartAddr(0x31010);
			SpuWrite(audiobuffer + 65536 * 3, 65536);
			SpuIsTransferCompleted(SPU_TRANSFER_WAIT);

			transferred_chunks = 2;
			target_chunk += 2;
			ContinueCD();

			SpuSetIRQAddr(0x31020);
			SpuSetIRQCallback((SpuIRQCallbackProc)spu_callback);
			SpuSetIRQ(SPU_ON);

			SpuSetVoiceStartAddr(0, 0x1010);
			SpuSetVoiceStartAddr(2, 0x21010);

			SpuSetVoiceVolume(0, MAX_VOLUME, 0);
			SpuSetVoiceVolume(2, 0, MAX_VOLUME);

			SpuSetKey(SPU_ON, SPU_0CH | SPU_2CH);

			playback_status = 1;
		}

		if(playback_status && SpuGetIRQ() == SPU_OFF && spu_transfer_progress == 0) {
			spu_transfer_progress = 1;
		}

		switch(spu_transfer_progress) {
			case 1:
				if(!SpuIsTransferCompleted(SPU_TRANSFER_PEEK)) break;

				// Load left channel data to SPU memory

				if(last_sector_id == 0xFFFF && transferred_chunks >= current_chunk) {
					playback_status = 2;
					break;
				}

				SpuSetTransferStartAddr((transferred_chunks & 1) ? 0x11010 : 0x1010);
				SpuWrite(audiobuffer + ((transferred_chunks & 3) << 17), 65536);

				spu_transfer_progress++;
				break;

			case 2:
				if(!SpuIsTransferCompleted(SPU_TRANSFER_PEEK)) break;

				// Load right channel data to SPU memory

				SpuSetTransferStartAddr((transferred_chunks & 1) ? 0x31010 : 0x21010);
				SpuWrite(audiobuffer + ((transferred_chunks & 3) << 17) + 65536, 65536);

				transferred_chunks++;
				spu_transfer_progress++;
				break;

			case 3:
				if(!SpuIsTransferCompleted(SPU_TRANSFER_PEEK)) break;

				// Load new chunk from disc, if necessary

				if(last_sector_id != 0xFFFF) {
					target_chunk++;

					if(!callback_running) ContinueCD();
				}

				spu_transfer_progress = 0;
				SpuSetIRQAddr((transferred_chunks & 1) ? 0x21020 : 0x31020);
				SpuSetIRQ(SPU_RESET);

				break;
		}

		padx = ParsePad(0, 0);

		if(padx & PADRdown) playback_status = 2;

		if(playback_status == 2) {
			SpuSetKey(SPU_OFF, SPU_0CH | SPU_2CH);
			StopCD();
			SpuSetIRQ(SPU_OFF);
			SpuSetIRQCallback(NULL);
			SpuIsTransferCompleted(SPU_TRANSFER_WAIT);
			VSyncCallback(NULL);
			return;
		}
    } while(1);
}

int main() {
	// Initialize system

	init();

	while(1) run_test();
}