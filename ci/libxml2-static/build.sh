#!/bin/bash

./configure --without-zlib --enable-shared=no --without-python --prefix=$PREFIX
make -j $CPU_COUNT
make install

# remove xml binaries
rm $PREFIX/bin/xmlcatalog $PREFIX/bin/xmllint

