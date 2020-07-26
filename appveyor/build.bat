if "%APPVEYOR_REPO_TAG_NAME%"=="continuous" (

    exit 1

)

git submodule init
git submodule update

set QTDIR=C:\Qt\5.15\msvc2019_64
set PATH=%PATH%;%QTDIR%\bin
call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64.bat"

rem Remove symlinks from contemporary-icons
cp appveyor\delink.ps1 application\icons\contemporary-icons
cd application\icons\contemporary-icons
powershell -executionpolicy bypass -File delink.ps1
cd ..\..\..

git clone https://github.com/taglib/taglib.git
cd taglib
mkdir build
cd build
cmake .. -G "NMake Makefiles" -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=ON -DENABLE_STATIC_RUNTIME=OFF
nmake
nmake install
cd ..\..

git clone https://github.com/vicr123/the-libs.git
cd the-libs
rem git checkout %APPVEYOR_REPO_BRANCH%
git checkout blueprint
qmake the-libs.pro "CONFIG+=release"
nmake release
nmake install
cd ..

git clone https://github.com/vicr123/contemporary-theme.git
cd contemporary-theme
qmake Contemporary.pro "CONFIG+=release"
nmake release
cd ..

qmake theBeat.pro "CONFIG+=release"
nmake release
mkdir deploy
mkdir deploy\styles
mkdir deploy\translations
mkdir deploy\icons
copy "contemporary-theme\release\Contemporary.dll" deploy\styles
copy application\release\thebeat.exe deploy
copy libthebeat\release\thebeat.dll deploy
copy application\translations\*.qm deploy\translations
robocopy application\icons\contemporary-icons deploy\icons\ /mir
copy "C:\Program Files\thelibs\lib\the-libs.dll" deploy
copy "C:\OpenSSL-Win64\bin\openssl.exe" deploy
copy "C:\OpenSSL-Win64\bin\libeay32.dll" deploy
copy "C:\OpenSSL-Win64\bin\ssleay32.dll" deploy
copy "C:\OpenSSL-Win64\bin\openssl.cfg" deploy
copy taglib\build\taglib\tag.dll deploy
cd deploy
windeployqt theBeat.exe -network -quickwidgets -sql -multimedia