#!/bin/sh

# Compile mkpak, if not already present

[ -f tools/mkpak ] || cc tools/mkpak.c -o tools/mkpak

# Convert the input to WAV

rm -f *.wav *.vag

ffmpeg -i "$1" -f wav -bitexact -acodec pcm_s16le -ar 44100 -map_channel 0.0.0 left.wav
ffmpeg -i "$1" -f wav -bitexact -acodec pcm_s16le -ar 44100 -map_channel 0.0.1 right.wav

echo "Converting left channel to VAG..."
wav2vag left.wav left.vag -raw

echo "Converting right channel to VAG..."
wav2vag right.wav right.vag -raw

echo "Generating final PAK..."
./tools/mkpak left.vag right.vag root/TEST.PAK

rm -f *.wav *.vag
echo "Done!"
