mkdir build
cd build

cmake \
    -DCMAKE_INSTALL_PREFIX:PATH=$PREFIX \
    ..

make -j ${CPU_COUNT}
make install

