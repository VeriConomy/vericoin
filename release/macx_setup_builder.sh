#!/bin/sh

# Note: The structure of this package depends on the -rpath,./lib to be set at compile/link time.

version="1.6"
arch=`uname -m`

if [ "${arch}" = "x86_64" ]; then
    arch="64bit"
else
    arch="32bit"
fi

if [ -f VeriCoin-Qt.app/Contents/MacOS/VeriCoin-Qt ] && [ -f vericoin.conf ] && [ -f README ]; then
    echo "Building VeriCoin_${version}_${arch}.pkg ...\n"
    cp vericoin.conf VeriCoin-Qt.app/Contents/MacOS/
    cp README VeriCoin-Qt.app/Contents/MacOS/

    # Remove the old archive
    if [ -f VeriCoin_${version}_${arch}.pkg ]; then
        rm -f VeriCoin_${version}_${arch}.pkg
    fi

    # Deploy the app, create the plist, then build the package.
    macdeployqt ./VeriCoin-Qt.app -always-overwrite
    pkgbuild --analyze --root ./VeriCoin-Qt.app share/qt/VeriCoin-Qt.plist
    pkgbuild --root ./VeriCoin-Qt.app --component-plist share/qt/VeriCoin-Qt.plist --identifier org.vericoin.VeriCoin-Qt --install-location /Applications/VeriCoin-Qt.app VeriCoin_${version}_${arch}.pkg
    echo "Package created in: $PWD/VeriCoin_${version}_${arch}.pkg\n"
else
    echo "Error: Missing files!\n"
    echo "Run this script from the folder containing VeriCoin-Qt.app, vericoin.conf and README.\n"
fi

