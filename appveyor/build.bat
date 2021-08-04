if "%APPVEYOR_REPO_TAG_NAME%"=="continuous" (

    exit 1

)

git submodule init
git submodule update

set QTDIR=C:\Qt\5.15\msvc2019_64
set PATH=%PATH%;%QTDIR%\bin
call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64.bat"

git clone https://github.com/discordapp/discord-rpc.git
cd discord-rpc
mkdir build
cd build
cmake -G"NMake Makefiles" .. -DBUILD_EXAMPLES=OFF -DCMAKE_POSITION_INDEPENDENT_CODE=ON -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release
cmake --build . --config Release --target install
cd ..
cd ..

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

git clone https://github.com/vicr123/theinstaller.git
cd theinstaller
git checkout blueprint
qmake theInstaller.pro "CONFIG+=applib"
nmake release
nmake install
cd ..

rem Build cdlib
cd plugins\WinLibCDPlugin\cdlib
nuget restore
msbuild -p:Configuration=Release -p:WindowsTargetPlatformVersion=10.0.18362.0
cd ..\..\..

qmake theBeat.pro "CONFIG+=release"
nmake release
mkdir deploy
mkdir deploy\styles
mkdir deploy\translations
mkdir deploy\icons
mkdir deploy\plugins
copy "contemporary-theme\release\Contemporary.dll" deploy\styles
copy application\release\thebeat.exe deploy
copy libthebeat\release\thebeat.dll deploy
copy application\translations\*.qm deploy\translations
robocopy application\icons\contemporary-icons deploy\icons\ /mir
copy "C:\Program Files\thelibs\lib\the-libs.dll" deploy
copy plugins\WinLibCDPlugin\cdlib\x64\Release\CDLib.dll deploy
copy plugins\WinLibCDPlugin\release\WinLibCDPlugin.dll deploy\plugins
copy plugins\DRPIntegration\release\DRPIntegration.dll deploy\plugins
copy plugins\InternetRadioPlugin\release\InternetRadioPlugin.dll deploy\plugins
copy "C:\OpenSSL-v111-Win64\bin\libssl-1_1-x64.dll" deploy
copy "C:\OpenSSL-v111-Win64\bin\libcrypto-1_1-x64.dll" deploy
copy taglib\build\taglib\tag.dll deploy
copy application\defaults.conf deploy
cd deploy
windeployqt theBeat.exe -network -quickwidgets -sql -multimedia

cd ..
robocopy deploy deployAppx /mir
cd deployAppx

robocopy ..\dist\win-pack\* . /mir
makepri createconfig /cf priconfig.xml /dq en-US
makepri new /pr . /cf priconfig.xml
makeappx pack /d . /p theBeat.msix