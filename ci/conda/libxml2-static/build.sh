#!/bin/bash

export CFLAGS="-fPIC $CFLAGS"
export CXXFLAGS="-fPIC $CXXFLAGS"
./configure --without-zlib --without-lzma --enable-shared=no --without-python --prefix=$PREFIX
make -j $CPU_COUNT
make install

# remove xml binaries
rm $PREFIX/bin/xmlcatalog $PREFIX/bin/xmllint

