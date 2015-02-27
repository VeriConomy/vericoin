#!/bin/sh

# This script depends on the GNU script makeself.sh found at: http://megastep.org/makeself/
# Note: The structure of this package depends on the -rpath,./lib to be set at compile/link time.

version="1.5.1"
arch=`uname -i`

if [ "${arch}" = "x86_64" ]; then
    arch="64bit"
    QtLIBPATH="${HOME}/Qt/5.4/gcc_64"
else
    arch="32bit"
    QtLIBPATH="${HOME}/Qt/5.4/gcc"
fi

if [ -f vericoin-qt ] && [ -f vericoin.conf ] && [ -f README ]; then
    echo "Building VeriCoin_${version}_${arch}.run ...\n"
    if [ -d VeriCoin_${version}_${arch} ]; then
        rm -fr VeriCoin_${version}_${arch}/
    fi
    mkdir VeriCoin_${version}_${arch}
    mkdir VeriCoin_${version}_${arch}/libs
    mkdir VeriCoin_${version}_${arch}/platforms
    mkdir VeriCoin_${version}_${arch}/imageformats
    cp vericoin-qt VeriCoin_${version}_${arch}/
    cp vericoin.conf VeriCoin_${version}_${arch}/
    cp README VeriCoin_${version}_${arch}/
    ldd vericoin-qt | grep libssl | awk '{ printf("%s\0", $3); }' | xargs -0 -I{} cp {} VeriCoin_${version}_${arch}/libs/
    ldd vericoin-qt | grep libdb_cxx | awk '{ printf("%s\0", $3); }' | xargs -0 -I{} cp {} VeriCoin_${version}_${arch}/libs/
    ldd vericoin-qt | grep libboost_system | awk '{ printf("%s\0", $3); }' | xargs -0 -I{} cp {} VeriCoin_${version}_${arch}/libs/
    ldd vericoin-qt | grep libboost_filesystem | awk '{ printf("%s\0", $3); }' | xargs -0 -I{} cp {} VeriCoin_${version}_${arch}/libs/
    ldd vericoin-qt | grep libboost_program_options | awk '{ printf("%s\0", $3); }' | xargs -0 -I{} cp {} VeriCoin_${version}_${arch}/libs/
    ldd vericoin-qt | grep libboost_thread | awk '{ printf("%s\0", $3); }' | xargs -0 -I{} cp {} VeriCoin_${version}_${arch}/libs/
    ldd vericoin-qt | grep libminiupnpc | awk '{ printf("%s\0", $3); }' | xargs -0 -I{} cp {} VeriCoin_${version}_${arch}/libs/
    ldd vericoin-qt | grep libqrencode | awk '{ printf("%s\0", $3); }' | xargs -0 -I{} cp {} VeriCoin_${version}_${arch}/libs/
    cp ${QtLIBPATH}/lib/libQt*.so.5 VeriCoin_${version}_${arch}/libs/
    cp ${QtLIBPATH}/lib/libicu*.so.53 VeriCoin_${version}_${arch}/libs/
    cp ${QtLIBPATH}/plugins/platforms/lib*.so VeriCoin_${version}_${arch}/platforms/
    cp ${QtLIBPATH}/plugins/imageformats/lib*.so VeriCoin_${version}_${arch}/imageformats/
    strip VeriCoin_${version}_${arch}/vericoin-qt
    echo "Enter your sudo password to change the ownership of the archive: "
    sudo chown -R nobody:nogroup VeriCoin_${version}_${arch}

    # now build the archive
    if [ -f VeriCoin_${version}_${arch}.run ]; then
        rm -f VeriCoin_${version}_${arch}.run
    fi
    makeself.sh --notemp VeriCoin_${version}_${arch} VeriCoin_${version}_${arch}.run "\nCopyright (c) 2014-2015 The VeriCoin Developers\nVeriCoin will start when the installation is complete...\n" ./vericoin-qt \&
    sudo rm -fr VeriCoin_${version}_${arch}/
    echo "Package created in: $PWD/VeriCoin_${version}_${arch}.run\n"
else
    echo "Error: Missing files!\n"
    echo "Copy this file to a setup folder along with vericoin-qt, vericoin.conf and README.\n"
fi

