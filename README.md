# VAGStream
An audio streaming library for the PlayStation.

## Why not use XA/CDDA streaming?
In case uninterrupted playback is absolutely essential.
This library will handle drive skips quite well – it is reading from disc only ~1/4 of the time (if no extra data is embedded in the stream),
and it keeps a large buffer (~10 seconds of 44.1 kHz stereo audio).

## Requirements
- 512 kB of system RAM for the main input buffer
- 256 kB of SPU RAM for the ring buffer (starting at addr 0x1010)

## Why?
Might need it for a future project.
