docker cp archbuild:/home/travis/appdir ~/appdir
ls ~/appdir

cd
wget -c -nv "https://github.com/probonopd/linuxdeployqt/releases/download/continuous/linuxdeployqt-continuous-x86_64.AppImage"
chmod a+x linuxdeployqt-continuous-x86_64.AppImage
echo "[TRAVIS] Building AppImage"
./linuxdeployqt-continuous-x86_64.AppImage ~/appdir/usr/share/applications/*.desktop -bundle-non-qt-libs -extra-plugins=iconengines/libqsvgicon.so,imageformats/libqsvg.so
./linuxdeployqt-continuous-x86_64.AppImage ~/appdir/usr/share/applications/*.desktop -appimage -extra-plugins=iconengines/libqsvgicon.so,imageformats/libqsvg.so

echo "[TRAVIS] Publishing AppImage"
wget -c https://github.com/probonopd/uploadtool/raw/master/upload.sh
bash upload.sh theBeat*.AppImage*
