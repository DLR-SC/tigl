mkdir build
cd build

REM Configure step
cmake -G "NMake Makefiles" -DCMAKE_INSTALL_PREFIX="%LIBRARY_PREFIX%" ^
 -DCMAKE_BUILD_TYPE=Release ^
 -DCMAKE_PREFIX_PATH="%LIBRARY_PREFIX%" ^
 -DCMAKE_SYSTEM_PREFIX_PATH="%LIBRARY_PREFIX%" ^
 ..
if errorlevel 1 exit 1

REM Build step 
nmake
if errorlevel 1 exit 1

REM remove linkage to static libs
cmake .

REM Install step
nmake install
if errorlevel 1 exit 1

REM install python packages
mkdir %SP_DIR%\tixi3
echo. 2> %SP_DIR%\tixi3\__init__.py
copy lib\tixi3wrapper.py %SP_DIR%\tixi3\
