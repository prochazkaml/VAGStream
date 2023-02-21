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

## Compilation instructions
Set up the PSY-Q SDK on Linux according to my own tutorial ([forum](http://www.psxdev.net/forum/viewtopic.php?f=53&t=3737), also archived on [my blog](https://blog.prochazkaml.eu/index.php/article/psyq-modern-linux) in case the forum decides to shut down again).

You'll also need to grab `upx`, `xxd` and `mednafen` - all are usually available straight from your repo.

Then, just run:

```
./make.sh
```

## How to create a custom PAK file
First, convert your audio file to WAV and split the channels. This can be done with `ffmpeg`:

```
ffmpeg -i song.ogg -f wav -bitexact -acodec pcm_s16le -ar 44100 -map_channel 0.0.0 left.wav
ffmpeg -i song.ogg -f wav -bitexact -acodec pcm_s16le -ar 44100 -map_channel 0.0.1 right.wav
```

Then, using `wav2vag` from [PSXSDK](https://github.com/simias/psxsdk/blob/master/tools/wav2vag.c), convert them to VAG (headerless):

```
wav2vag left.wav left.vag -raw
wav2vag right.wav right.vag -raw
```

Finally, compile the `mkpak` tool in the tools/ directory and run it:

```
cc tools/mkpak.c -o tools/mkpak
./tools/mkpak left.vag right.vag root/TEST.PAK
```

Compile, and you're done!
