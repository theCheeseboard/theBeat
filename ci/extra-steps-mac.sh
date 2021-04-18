# Modify the AVFoundation plugin
install_name_tool -change libthe-libs.1.dylib @executable_path/../Libraries/libthe-libs.1.dylib build/application/theBeat.app/Contents/AppPlugins/libAvFoundationPlugin.dylib 
install_name_tool -change libthebeat.1.dylib @executable_path/../Libraries/libthebeat.1.dylib build/application/theBeat.app/Contents/AppPlugins/libAvFoundationPlugin.dylib

# Modify the Mac Integration plugin
install_name_tool -change libthe-libs.1.dylib @executable_path/../Libraries/libthe-libs.1.dylib build/application/theBeat.app/Contents/AppPlugins/libMacIntegration.dylib 
install_name_tool -change libthebeat.1.dylib @executable_path/../Libraries/libthebeat.1.dylib build/application/theBeat.app/Contents/AppPlugins/libMacIntegration.dylib

# Modify the Internet Radio plugin
install_name_tool -change libthe-libs.1.dylib @executable_path/../Libraries/libthe-libs.1.dylib build/application/theBeat.app/Contents/AppPlugins/libInternetRadioPlugin.dylib 
install_name_tool -change libthebeat.1.dylib @executable_path/../Libraries/libthebeat.1.dylib build/application/theBeat.app/Contents/AppPlugins/libInternetRadioPlugin.dylib