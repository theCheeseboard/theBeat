set -e
sudo pacman -S --noconfirm git qt5-base qt5-charts qt5-location qt5-tools qt5-svg phonon-qt5 taglib wget fuse2

git clone https://aur.archlinux.org/the-libs.git
cd the-libs
makepkg -i --noconfirm
cd ..

git clone https://github.com/vicr123/thebeat.git
cd thebeat
git checkout blueprint
qmake
make
make install INSTALL_ROOT=~/appdir
wget -c -nv "https://github.com/probonopd/linuxdeployqt/releases/download/continuous/linuxdeployqt-continuous-x86_64.AppImage"
chmod a+x linuxdeployqt-continuous-x86_64.AppImage
echo "[TRAVIS] Building AppImage"
./linuxdeployqt-continuous-x86_64.AppImage ~/appdir/usr/share/applications/*.desktop -bundle-non-qt-libs -extra-plugins=iconengines/libqsvgicon.so,imageformats/libqsvg.so
./linuxdeployqt-continuous-x86_64.AppImage ~/appdir/usr/share/applications/*.desktop -appimage -extra-plugins=iconengines/libqsvgicon.so,imageformats/libqsvg.so
cp theBeat*.AppImage* ~
