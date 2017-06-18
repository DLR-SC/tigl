#!/bin/bash

if [ `uname` == Darwin ]; then
    EXTRA_LIBS="--with-darwinssl"
else
    EXTRA_LIBS=""
fi

CFLAGS=-fPIC CXXFLAGS=-fPIC ./configure  "$EXTRA_LIBS"" --enable-shared=no --without-zlib --disable-ldap --prefix=$PREFIX
make -j $CPU_COUNT
make install
