./buildassets.sh

export WINEPREFIX=$HOME/.wine/32bit

wineconsole tools/make32.bat

mv main.exe ./root/MAIN.EXE
mkpsxiso -y build.xml

./run.sh
