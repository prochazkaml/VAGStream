#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

void patch_loop(uint8_t *data, int chunktype) {
	int i;

	data[1] = chunktype ? 2 : 6;	// Loop start (if type 0)

	for(i = 17; i < 65536 - 16; i += 16) {
		data[i] = 2;	// Normal playback
	}

	data[i] = chunktype ? 3 : 2;		// Loop back to the start (if type 1 - to create a circular buffer)
}

int main(int argc, char *argv[]) {
	FILE *left = fopen(argv[1], "rb");
	FILE *right = fopen(argv[2], "rb");
	FILE *out = fopen(argv[3], "wb");

	int chunks = 0;

	uint8_t *chunkhdr = malloc(2048);
	uint8_t *chunkdata = malloc(65536);

	chunkhdr[2] = 0x04;
	chunkhdr[3] = 0x00;

	int chunktype = 0;

	int lsize = 1, rsize = 1;

	while(lsize == 1 && rsize == 1) {
		chunkhdr[0] = chunks;
		chunkhdr[1] = chunks >> 8;

		fwrite(chunkhdr, 2048, 1, out);

		memset(chunkdata, 0, 65536);
		lsize = fread(chunkdata, 65536, 1, left);
		patch_loop(chunkdata, chunktype);
		fwrite(chunkdata, 65536, 1, out);

		memset(chunkdata, 0, 65536);
		rsize = fread(chunkdata, 65536, 1, right);
		patch_loop(chunkdata, chunktype);
		fwrite(chunkdata, 65536, 1, out);

		chunks++;
		chunktype ^= 1;
	}

	chunkhdr[0] = chunkhdr[1] = 0xFF;

	fwrite(chunkhdr, 2048, 1, out);

	fclose(left);
	fclose(right);
	fclose(out);
}
