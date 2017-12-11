
# To make libTIXI.so back-portable, we have to use clock_gettime
# from librt instead of glibc. Thus we have to link rt explicitly

mkdir build
cd build

if [ `uname` == Darwin ]; then
    EXTRA_LIBS="-lm -liconv -framework Foundation -lz -framework Security"
else
    EXTRA_LIBS="-lm -lrt"
fi

# Configure step
cmake -DCMAKE_INSTALL_PREFIX=$PREFIX \
 -DCMAKE_BUILD_TYPE=Release \
 -DCMAKE_PREFIX_PATH=$PREFIX \
 -DCMAKE_SYSTEM_PREFIX_PATH=$PREFIX \
 -DCMAKE_SHARED_LINKER_FLAGS="$EXTRA_LIBS" \
 -DCMAKE_EXE_LINKER_FLAGS="$EXTRA_LIBS" \
 -DTIXI_BUILD_TESTS=ON \
 -DPYTHON_EXECUTABLE:FILEPATH=$PYTHON \
 ..

# Build step
make -j $CPU_COUNT
make doc

# remove linkage to static libs
cmake .

# Install step
make install

# Tests
# make test

# create the binary package
make package
cp *.tar.gz $RECIPE_DIR/

# install python packages
mkdir $SP_DIR/tixi3
touch $SP_DIR/tixi3/__init__.py
cp lib/tixi3wrapper.py $SP_DIR/tixi3/
python $RECIPE_DIR/fixosxload.py $SP_DIR/tixi3/tixi3wrapper.py libTIXI
