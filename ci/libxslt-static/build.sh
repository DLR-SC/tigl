#!/bin/bash

./configure --without-crypto --enable-shared=no --without-python --prefix=$PREFIX
make -j $CPU_COUNT
make install

rm $PREFIX/bin/xsltproc
