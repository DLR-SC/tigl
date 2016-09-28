mkdir build
cd build

# Configure step
cmake -DCMAKE_INSTALL_PREFIX=$PREFIX \
 -DCMAKE_BUILD_TYPE=Release \
 -DCMAKE_PREFIX_PATH=$PREFIX \
 -DCMAKE_SYSTEM_PREFIX_PATH=$PREFIX \
 ..

# Build step
make -j $CPU_COUNT

# Install step
make install

# install python packages
mkdir $SP_DIR/tixi
touch $SP_DIR/tixi/__init__.py
cp lib/tixiwrapper.py $SP_DIR/tixi/
python $RECIPE_DIR/fixosxload.py $SP_DIR/tixi/tixiwrapper.py libTIXI
