mkdir build
cd build

REM Configure step
cmake -G "Ninja" -DCMAKE_INSTALL_PREFIX="%LIBRARY_PREFIX%" ^
 -DCMAKE_BUILD_TYPE=Release ^
 -DOCE_NO_LIBRARY_VERSION=ON ^
 -DOCE_BUILD_SHARED_LIB=OFF ^
 -DOCE_TESTING=OFF ^
 -DOCE_USE_PCH=OFF ^
 -DCMAKE_PREFIX_PATH="%LIBRARY_PREFIX%" ^
 -DCMAKE_SYSTEM_PREFIX_PATH="%LIBRARY_PREFIX%" ^
 -DOCE_INSTALL_LIB_DIR=lib ^
 -DOCE_INSTALL_BIN_DIR=bin ^
 -DOCE_MULTITHREAD_LIBRARY=OPENMP ^
 -DOCE_WITH_FREEIMAGE=ON ^
 -DOCE_INSTALL_PREFIX="%LIBRARY_PREFIX%" -DOCE_ENABLE_DEB_FLAG=OFF ..
if errorlevel 1 exit 1
 
REM Build step 
ninja
if errorlevel 1 exit 1

REM Install step
ninja install
if errorlevel 1 exit 1


REM Fix hardcoded absolute freetype and freeimage paths
python %RECIPE_DIR%\fixpaths.py "%LIBRARY_PREFIX%\cmake\OCE-libraries-release.cmake" "%LIBRARY_PREFIX%"\lib\
if errorlevel 1 exit 1
