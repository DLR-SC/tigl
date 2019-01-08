mkdir build
cd build

REM Configure step
cmake -G "NMake Makefiles" -DCMAKE_INSTALL_PREFIX="%LIBRARY_PREFIX%" ^
 -DCMAKE_BUILD_TYPE=Release ^
 -DCMAKE_PREFIX_PATH="%LIBRARY_PREFIX%" ^
 -DBUILD_SHARED_LIBS=ON ^
 -DCMAKE_SYSTEM_PREFIX_PATH="%LIBRARY_PREFIX%" ^
 -DLIBXML2_LIBRARY="%LIBRARY_PREFIX%\lib\libxml2_a.lib" ^
 -DLIBXSLT_LIBRARIES="%LIBRARY_PREFIX%\lib\libxslt_a.lib" ^
 ..
if errorlevel 1 exit 1

REM Build step 
nmake
if errorlevel 1 exit 1

REM Install step
nmake install
if errorlevel 1 exit 1

REM install python packages
mkdir %SP_DIR%\tixi3
echo. 2> %SP_DIR%\tixi3\__init__.py
copy lib\tixi3wrapper.py %SP_DIR%\tixi3\
