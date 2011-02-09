#!/bin/bash

# create diretory for tar ball
rm -rf TIGL
mkdir TIGL
mkdir TIGL/doc
mkdir TIGL/lib
mkdir TIGL/include


# build static TIGL
cd Src
make clean
make -f Makefile
cp libTIGL.a ../TIGL/lib

# build shared TIGL
make -f Makefile.shared clean
make -f Makefile.shared
cp libTIGL.so ../TIGL/lib
cd ..

# make doc
doxygen Doc/Doxyfile

# Copy all to one dir
cp -r Doc/html TIGL/doc
cp Src/tigl.h TIGL/include

#pack
rm TIGL_*.tar.gz
tar cvfz TIGL-0.X_linux_gcc3_static_shared_`date +%y-%m-%d`.tar.gz TIGL


