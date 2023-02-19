#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

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

	fseek(left, 0, SEEK_END);
	int size = ftell(left);
	fseek(left, 0, SEEK_SET);

	fseek(right, 0, SEEK_END);
	int rsize = ftell(right);
	fseek(right, 0, SEEK_SET);

	if(size != rsize) {
		printf("ERROR: Channel data size mismatch!");
		exit(1);
	}

	int chunks = 0;

	uint8_t *chunkhdr = malloc(2048);
	uint8_t *chunkdata = malloc(65536);

	chunkhdr[2] = 0x04;
	chunkhdr[3] = 0x00;

	int chunktype = 0;

	while(size > 0) {
		chunkhdr[0] = chunks;
		chunkhdr[1] = chunks >> 8;

		fwrite(chunkhdr, 2048, 1, out);

		fread(chunkdata, 65536, 1, left);
		patch_loop(chunkdata, chunktype);
		fwrite(chunkdata, 65536, 1, out);

		fread(chunkdata, 65536, 1, right);
		patch_loop(chunkdata, chunktype);
		fwrite(chunkdata, 65536, 1, out);

		chunks++;
		size -= 65536;
		chunktype ^= 1;
	}

	chunkhdr[0] = chunkhdr[1] = 0xFF;

	fwrite(chunkhdr, 2048, 1, out);

	fclose(left);
	fclose(right);
	fclose(out);
}
