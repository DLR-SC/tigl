mkdir build
cd build

REM Configure step
cmake -G "Ninja" .. ^
      -D CMAKE_PREFIX_PATH:FILEPATH="%LIBRARY_PREFIX%" ^
      -D CMAKE_INSTALL_PREFIX:FILEPATH="%LIBRARY_PREFIX%" ^
      -D INSTALL_DIR_LAYOUT="Unix" ^
      -D BUILD_MODULE_Draw=OFF ^
      -D 3RDPARTY_DIR:FILEPATH="%LIBRARY_PREFIX%" ^
      -D CMAKE_BUILD_TYPE="Release" ^
      -D USE_TBB=ON ^
      -D USE_FREEIMAGE=ON ^
      -D BUILD_LIBRARY_TYPE=Static ^
      -D BUILD_RELEASE_DISABLE_EXCEPTIONS=OFF

if errorlevel 1 exit 1

REM Build step 
cmake --build .
if errorlevel 1 exit 1

REM Install step
cmake --build . --target install
if errorlevel 1 exit 1
