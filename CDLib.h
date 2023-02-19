typedef struct {
	char *filename; 
	int startpos;
} CDFILE;

CDFILE archive = { "\\TEST.PAK;1", 0 };

unsigned char databuffer[16384]; // Should be plenty
unsigned char databufferid;

unsigned char audiobuffer[524288]; // Buffer for 256 sectors, how convenient
unsigned char audiobufferid;
unsigned char lastsentaudiobuffer;

int current_chunk, target_chunk;
int callback_running, sectors_read, last_sector_id, remaining_data_sectors, remaining_audio_sectors;

void PrepareCD();
void cbready(int intr, u_char *result);
void UnprepareCD();
void PlayCD();
void StopCD();

void PlayCD() {
	CdlFILE fp;
	CdlLOC loc;

	StopCD();

	CdSearchFile(&fp, archive.filename);
	archive.startpos = CdPosToInt(&fp.pos);

	PrepareCD();

	// starting position on the CD
	CdIntToPos(archive.startpos, &loc);

	// begin playing
	CdControlF(CdlReadN, (u_char *)&loc);
}

void ContinueCD() {
	CdlLOC loc;

	// starting position on the CD
	CdIntToPos(archive.startpos, &loc);

	// begin playing
	CdControlF(CdlReadN, (u_char *)&loc);

	callback_running = 1;
}

void StopCD() {
	UnprepareCD();
}

void PrepareCD() {
	u_char param[4];

	param[0] = CdlModeSpeed; // 2X speed

	CdControlB(CdlSetmode, param, 0);
	CdControlF(CdlPause,0);

	callback_running = 1;
	CdReadyCallback((CdlCB)cbready);
}

void UnprepareCD() {
	u_char param[4];

    // reset any callback that we replaced
	CdControlF(CdlPause,0);
	CdReadyCallback(NULL);

	param[0] = 0;
	CdControlB(CdlSetmode, param, 0);
}

void cbready(int intr, u_char *result) {
	unsigned short id, len;

	void *buffer;

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

					archive.startpos += 1 + remaining_data_sectors + remaining_audio_sectors;

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
