#!/bin/bash

./configure --without-ssl --enable-shared=no --without-zlib --prefix=$PREFIX
make -j $CPU_COUNT
make install
