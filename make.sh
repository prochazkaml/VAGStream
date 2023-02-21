./buildassets.sh

export WINEPREFIX=$HOME/.psyq

wineconsole tools/make32.bat

upx main.exe
mv main.exe ./root/MAIN.EXE
mkpsxiso -y build.xml

./run.sh
