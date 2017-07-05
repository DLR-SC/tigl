#!/bin/bash

CFLAGS=-fPIC CXXFLAGS=-fPIC ./configure --without-ssl --enable-shared=no --without-zlib --prefix=$PREFIX
make -j $CPU_COUNT
make install
