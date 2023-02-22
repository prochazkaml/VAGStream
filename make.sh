#!/bin/sh

if [ ! -f "root/TEST.PAK" ]; then
	echo "root/TEST.PAK does not exist!"
	echo "Please run \"./tools/mkpak.sh path_to_audio.mp3\" to generate it."
	exit 1
fi

./buildassets.sh

export WINEPREFIX=$HOME/.psyq

wineconsole tools/make32.bat

upx main.exe
mv main.exe ./root/MAIN.EXE
mkpsxiso -y build.xml

./run.sh
