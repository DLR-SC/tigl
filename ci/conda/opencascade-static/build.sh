
declare -a CMAKE_PLATFORM_FLAGS
if [[ ${HOST} =~ .*linux.* ]]; then
    CMAKE_PLATFORM_FLAGS+=(-DCMAKE_TOOLCHAIN_FILE="${RECIPE_DIR}/cross-linux.cmake")
fi


mkdir -p build
cd build

echo "HERE: $SRC_DIR" ${CMAKE_PLATFORM_FLAGS[@]}

cmake -G "Ninja" .. \
      -DCMAKE_BUILD_TYPE=Release \
       ${CMAKE_PLATFORM_FLAGS[@]} \
      -D CMAKE_INSTALL_PREFIX:FILEPATH=$PREFIX \
      -D CMAKE_PREFIX_PATH:FILEPATH=$PREFIX \
      -D 3RDPARTY_DIR:FILEPATH=$PREFIX \
      -D BUILD_MODULE_Draw:BOOL=OFF \
      -D USE_TBB:BOOL=ON \
      -D USE_FREEIMAGE:BOOL=ON \
      -D BUILD_LIBRARY_TYPE=Static \
      -D BUILD_RELEASE_DISABLE_EXCEPTIONS=OFF \

# Build step
cmake --build . 

# Install step
cmake --build .  --target install
