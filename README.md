<img src="readme/splash.svg" width="100%" />

---
<p align="center">
<img src="https://img.shields.io/github/v/release/vicr123/thebeat?label=LATEST&style=for-the-badge">
<img src="https://img.shields.io/github/license/vicr123/thebeat?style=for-the-badge" />
</p>

theBeat is an audio player.

---

## Dependencies
- Qt 6
  - Qt Core
  - Qt GUI
  - Qt Widgets
  - Qt Multimedia
  - Qt SVG
- [the-libs](https://github.com/vicr123/the-libs)
- [taglib](https://taglib.org/)

### Additional macOS Dependencies
- (optional) [libmusicbrainz](https://musicbrainz.org/doc/libmusicbrainz) for CDDB queries

### Additional Linux Dependencies
- (optional) [phonon4qt5](https://invent.kde.org/libraries/phonon) for playing CDs
- (optional) [cdrdao](http://cdrdao.sourceforge.net/) for burning CDs
- (optional) [libmusicbrainz](https://musicbrainz.org/doc/libmusicbrainz) for CDDB queries

## Build
Run the following commands in your terminal. 
```
mkdir build
qmake ../theBeat.pro
make
```

## Install
On Linux, run the following command in your terminal (with superuser permissions)
```
make install
```

## Docs
For help with theBeat, visit the [theBeat Documentation](https://help.vicr123.com/docs/thebeat/intro).

---

> © Victor Tran, 2022. This project is licensed under the GNU General Public License, version 3, or at your option, any later version.
> 
> Check the [LICENSE](LICENSE) file for more information.
