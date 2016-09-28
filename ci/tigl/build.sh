mkdir build
cd build

if [ "$PY3K" == "1" ]; then
    MY_PY_VER="${PY_VER}m"
else
    MY_PY_VER="${PY_VER}"
fi

if [ `uname` == Darwin ]; then
    PY_LIB="libpython${MY_PY_VER}.dylib"
else
    PY_LIB="libpython${MY_PY_VER}.so"
fi

# Configure step
cmake -G "Ninja" -DCMAKE_INSTALL_PREFIX=$PREFIX \
 -DCMAKE_BUILD_TYPE=Release \
 -DCMAKE_PREFIX_PATH=$PREFIX \
 -DCMAKE_SYSTEM_PREFIX_PATH=$PREFIX \
 -DTIGL_VIEWER=ON \
 -DPYTHON_EXECUTABLE:FILEPATH=$PYTHON \
 -DTIGL_BUILD_TESTS=ON \
 ..


# Build step
ninja

# Install step
ninja install

# Create the packages
ninja package
cp *.tar.gz *.dmg $RECIPE_DIR\

# install python packages
mkdir -p $SP_DIR/tigl
touch $SP_DIR/tigl/__init__.py
cp lib/tiglwrapper.py $SP_DIR/tigl/
python $RECIPE_DIR/fixosxload.py $SP_DIR/tigl/tiglwrapper.py libTIGL
