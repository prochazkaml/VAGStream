#include "StreamLib.h"
#include "FontLib.h"
#include "SysLib.h"

POLY_F4 buffermeter;

unsigned char databuffer[16384]; // Should be plenty
unsigned char databufferid;

unsigned char audiobuffer[524288]; // Buffer for 256 sectors, how convenient
unsigned char audiobufferid;

int current_chunk, target_chunk;
int callback_running, sectors_read, last_sector_id, remaining_data_sectors, remaining_audio_sectors;
int filepos;

int status = 0;

int transferred_chunks;

int checksum;

void cbready(int intr, u_char *result);
int StartStream(char *filename);
int ProcessStream();
void StopStream();
void UnprepareCD();
void ContinueCD();

SpuIRQCallbackProc spu_callback() {
	SpuSetIRQ(SPU_OFF);
}

int StartStream(char *filename) {
	CdlFILE fp;
	CdlLOC loc;
	int i;
	u_char param[4] = { CdlModeSpeed };

	SetPolyF4(&buffermeter);
	setRGB0(&buffermeter, 255, 255, 255);

	UnprepareCD();

	databufferid = audiobufferid = 0;
	current_chunk = 0;
	target_chunk = 4;		// Fill the entire buffer

	sectors_read = 0;
	last_sector_id = 0;
	remaining_data_sectors = 0;
	remaining_audio_sectors = 0;

	transferred_chunks = 0;
	checksum = 0;

	// Search for the requested file

	if(CdSearchFile(&fp, filename) == NULL) {
		status = 1;
		return 1;
	}

	filepos = CdPosToInt(&fp.pos);

	status = 2;

	// Initialize the CD subsystem to run at 2X speed

	CdControlB(CdlSetmode, param, 0);
	CdControlF(CdlPause, 0);

	// Start the CD callback

	callback_running = 1;
	CdReadyCallback((CdlCB)cbready);

	// Set the file starting position on the CD

	CdIntToPos(filepos, &loc);

	// Fill the initial buffer

	CdControlF(CdlReadN, (u_char *)&loc);

	while(callback_running) VSync(0);

	// Calculate the checksum (optional)

	for(i = 0; i < 131072 * 4; i++) {
		checksum += audiobuffer[i];
	}

	// Transfer the initial buffer to the SPU

	SpuSetKey(SPU_OFF, SPU_0CH | SPU_1CH);

	SpuIsTransferCompleted(SPU_TRANSFER_WAIT);	// Just in case

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

	// Already replace the transferred buffers

	target_chunk += 2;
	ContinueCD();

	// Set SPU IRQ to detect when to feed the SPU's RAM more data

	SpuSetIRQAddr(0x31020);
	SpuSetIRQCallback((SpuIRQCallbackProc)spu_callback);
	SpuSetIRQ(SPU_ON);

	// Start the SPU stream on channels 0 (left) and 1 (right)

	SpuSetVoiceStartAddr(0, 0x1010);
	SpuSetVoiceStartAddr(1, 0x21010);

	SpuSetVoiceVolume(0, MAX_VOLUME, 0);
	SpuSetVoiceVolume(1, 0, MAX_VOLUME);

	SpuSetKey(SPU_ON, SPU_0CH | SPU_1CH);

	return 0;
}

int ProcessStream() {
	if(SpuGetIRQ() == SPU_OFF) {
		SpuIsTransferCompleted(SPU_TRANSFER_WAIT);	// Just in case

		// Load left channel data to SPU memory

		if(last_sector_id == 0xFFFF && (transferred_chunks - 1) >= current_chunk) {
			return 1;
		}

		SpuSetTransferStartAddr((transferred_chunks & 1) ? 0x11010 : 0x1010);
		SpuWrite(audiobuffer + ((transferred_chunks & 3) << 17), 65536);
		SpuIsTransferCompleted(SPU_TRANSFER_WAIT);

		// Load right channel data to SPU memory

		SpuSetTransferStartAddr((transferred_chunks & 1) ? 0x31010 : 0x21010);
		SpuWrite(audiobuffer + ((transferred_chunks & 3) << 17) + 65536, 65536);
		SpuIsTransferCompleted(SPU_TRANSFER_WAIT);

		transferred_chunks++;

		// Load new chunk from disc, if necessary

		if(last_sector_id != 0xFFFF) {
			target_chunk++;

			if(!callback_running) ContinueCD();
		}

		SpuSetIRQAddr((transferred_chunks & 1) ? 0x21020 : 0x31020);
		SpuSetIRQ(SPU_RESET);
	}

	return 0;
}

void StopStream() {
	SpuSetKey(SPU_OFF, SPU_0CH | SPU_1CH);
	UnprepareCD();
	SpuSetIRQ(SPU_OFF);
	SpuSetIRQCallback(NULL);
	SpuIsTransferCompleted(SPU_TRANSFER_WAIT);

	status = 0;
}

void ContinueCD() {
	CdlLOC loc;

	// starting position on the CD
	CdIntToPos(filepos, &loc);

	// begin playing
	CdControlF(CdlReadN, (u_char *)&loc);

	callback_running = 1;
}

void UnprepareCD() {
	u_char param[4];

    // reset any callback that we replaced
	CdControlF(CdlPause,0);
	CdReadyCallback(NULL);
	callback_running = 0;

	param[0] = 0;
	CdControlB(CdlSetmode, param, 0);
}

void cbready(int intr, u_char *result) {
	unsigned short id, len;
	void *buffer;

	if(last_sector_id == 0xFFFF) {
		// If the program tries to do anything stupid, stop it immediately

		CdControlF(CdlPause, 0);
		callback_running = 0;
		return;
	}

	if (intr == CdlDataReady) {
		if(remaining_data_sectors) {
			// Read data sector

			buffer = databuffer + ((databufferid++) << 11);

			CdGetSector((u_long*)buffer, 512); // 2048 bytes

			remaining_data_sectors--;
		} else if(remaining_audio_sectors) {
			// Read audio sector

			buffer = audiobuffer + ((audiobufferid++) << 11);

			CdGetSector((u_long*)buffer, 512); // 2048 bytes

			remaining_audio_sectors--;
		} else {
			// Read data header sector

			if(current_chunk < target_chunk) {
				buffer = databuffer + ((databufferid++) << 11);

				CdGetSector((u_long*)buffer, 512); // 2048 bytes

				last_sector_id = id = *(unsigned short *)(buffer);

				if(id == 0xFFFF) {	// Terminator ID
					CdControlF(CdlPause, 0);

					callback_running = 0;
				} else {
					remaining_data_sectors = len = (*(unsigned short *)(buffer + 2)) >> 11;
					remaining_audio_sectors = 131072 >> 11;

					filepos += 1 + remaining_data_sectors + remaining_audio_sectors;

					current_chunk++;
				}
			} else {
				CdControlF(CdlPause, 0);

				callback_running = 0;
				sectors_read--;
			}
		}

		databufferid &= 7;

		sectors_read++;
	}
}

void StreamDebugScreen() {
	char buffer[64];
	int i;
	
	Font_ChangeColor(255, 255, 255);

	Font_ChangePosition(0, -240);
	Font_PrintStringCentered("CD Stream Debug Info");

	sprintf(buffer, "%d I'm not dead", VSync(-1));
	Font_ChangePosition(0, 240 - 32);
	Font_PrintStringCentered(buffer);

	switch(status) {
		case 0:
			Font_ChangePosition(0, -16);
			Font_PrintStringCentered("No stream is running");
			break;

		case 1:
			Font_ChangePosition(0, -16);
			Font_PrintStringCentered("Error opening stream!");
			break;

		case 2:
			i = 320 * ((current_chunk - transferred_chunks) * 64 - remaining_audio_sectors) / 256;
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
			break;
	}
}
