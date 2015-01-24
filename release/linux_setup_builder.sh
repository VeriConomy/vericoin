#!/bin/sh

version="1.5"
arch=`uname -i`

if [ "${arch}" = "x86_64" ]; then
    arch="64bit"
else
    arch="32bit"
fi

if [ ! -f vericoin-qt ]; then
    cd ..
fi

if [ -f vericoin-qt ] & [ -f vericoin.conf ] & [ -f README ]; then
    echo "Building VeriCoin_${version}_${arch}.zip ...\n"
    mkdir VeriCoin_${version}_${arch}
    cp vericoin-qt VeriCoin_${version}_${arch}/
    cp vericoin.conf VeriCoin_${version}_${arch}/
    cp README VeriCoin_${version}_${arch}/
    strip VeriCoin_${version}_${arch}/vericoin-qt
    zip -r VeriCoin_${version}_${arch}.zip VeriCoin_${version}_${arch}
    rm -fr VeriCoin_${version}_${arch}/
    echo "Package created in: $PWD/VeriCoin_${version}_${arch}.zip\n"
else
    echo "Error: Missing one of: vericoin-qt vericoin.conf README\n"
fi
