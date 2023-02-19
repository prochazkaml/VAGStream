#define PAD_REPEAT_DELAY 25
#define PAD_REPEAT_RATE 5

int padcounter[16] = { 0 };

int ParsePad(int oneshotkeys, int holdkeys) {
	int i, retval = 0, pad = PadRead(0);

	for(i = 0; i < 16; i++) {
		if(pad & (1 << i)) {
			if(holdkeys & (1 << i)) {
				retval |= (1 << i);
				continue;
			}

			if(padcounter[i] == 0 || padcounter[i] == PAD_REPEAT_DELAY) {
				retval |= (1 << i);
			}

			padcounter[i]++;

			if(oneshotkeys & (1 << i) && padcounter[i] > 1)
				padcounter[i] = 1;

			if(padcounter[i] >= (PAD_REPEAT_DELAY + PAD_REPEAT_RATE)) {
				padcounter[i] = PAD_REPEAT_DELAY;
			}

		} else {
			padcounter[i] = 0;
		}
	}

	return retval;
}