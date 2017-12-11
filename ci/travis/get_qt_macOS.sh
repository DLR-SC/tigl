#!/bin/bash

# download qt
echo "Downloading Qt5.4"
/usr/bin/curl -o Qt5.4_clang64_macOS.tar.gz -L https://sourceforge.net/projects/tigl/files/Thirdparty/Qt5.4_clang64_macOS.tar.gz

# extract
echo "Extracting Qt"
tar xf Qt5.4_clang64_macOS.tar.gz

# make relocatable, we require the qt online installer to do this
/usr/bin/curl -o qt-opensource-mac-x64-1.6.0-8-online.dmg -L http://download.qt.io/archive/online_installers/1.6/qt-opensource-mac-x64-1.6.0-8-online.dmg

# mount the installer
hdiutil attach qt-opensource-mac-x64-1.6.0-8-online.dmg

# run the installer with the proper options
echo "Make Qt relocatable"
/Volumes/qt-opensource-mac-x64-1.6.0-online/qt-opensource-mac-x64-1.6.0-online.app/Contents/MacOS/qt-opensource-mac-x64-1.6.0-online --runoperation "QtPatch" "mac" "`pwd`/Qt5.4_clang64_macOS" "qt5"
