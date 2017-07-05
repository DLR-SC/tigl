#!/bin/bash

# Build step 
make -j $CPU_COUNT -f Makefile.fip libfreeimageplus.a

mkdir $PREFIX/include
cp Source/FreeImage.h $PREFIX/include
cp Wrapper/FreeImagePlus/FreeImagePlus.h $PREFIX/include
mkdir $PREFIX/lib
cp *.a $PREFIX/lib
cd $PREFIX/lib
ln -s libfreeimageplus.a libfreeimage.a

