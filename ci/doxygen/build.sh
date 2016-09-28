mkdir build
cd build

cmake \
    -DCMAKE_INSTALL_PREFIX:PATH=$PREFIX \
    -DCMAKE_PREFIX_PATH=$PREFIX \
    -DCMAKE_SYSTEM_PREFIX_PATH=$PREFIX \
    ..

make -j ${CPU_COUNT}
make install

