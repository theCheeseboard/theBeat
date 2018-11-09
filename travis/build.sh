set -e
sudo pacman -S --noconfirm git qt5-base qt5-charts qt5-location qt5-tools phonon-qt5 taglib wget fuse2 fuse3

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
