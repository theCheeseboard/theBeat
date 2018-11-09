cd
echo "[TRAVIS] Publishing AppImage"
wget -c https://github.com/probonopd/uploadtool/raw/master/upload.sh
bash upload.sh theBeat*.AppImage*
