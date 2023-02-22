#include <SYS/TYPES.H>
#include <KERNEL.H>
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

#include "FontLib.h"
#include "SysLib.h"
#include "3DLib.h"
#include "ThreadLib.h"
#include "StreamLib.h"
#include "InputLib.h"

// TODO: Handle drive errors, buffer underruns, open drive trays!

int control = 0;
	
void debug_screen() {
	char buffer[64];
	int i, padx;

	PrepDisplay();

	StreamDebugScreen();

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

	while(1) {
		InitSubThread(subthread);

		VSyncCallback(ReturnToMainThread);

		control = 0;

		while(control < 2) {
			debug_screen();

			padx = ParsePad(0, 0);

			if(padx & PADRdown) control = 1;

			RunSubThread();
		}

		VSyncCallback(NULL);

		StopSubThread();
	}
}