# Modify the AVFoundation plugin
install_name_tool -change build/application/theBeat.app/Contents/AppPlugins/libAvFoundationPlugin.dylib libthe-libs.1.dylib @executable_path/../Libraries/libthe-libs.1.dylib
install_name_tool -change build/application/theBeat.app/Contents/AppPlugins/libAvFoundationPlugin.dylib libthebeat.1.dylib @executable_path/../Libraries/libthebeat.1.dylib
