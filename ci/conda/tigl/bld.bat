if exist cmake\NSIS.template.in (
    del cmake\NSIS.template.in /Q /F
)

if exist cmake\InstallRequiredSystemLibraries.cmake (
    del cmake\InstallRequiredSystemLibraries.cmake /Q /F
)

mkdir build
cd build

REM Remove dot from PY_VER for use in library name
set MY_PY_VER=%PY_VER:.=%
REM Configure step
cmake -G "Ninja" -DCMAKE_INSTALL_PREFIX="%LIBRARY_PREFIX%" ^
 -DCMAKE_BUILD_TYPE=Release ^
 -DCMAKE_PREFIX_PATH="%LIBRARY_PREFIX%" ^
 -DCMAKE_SYSTEM_PREFIX_PATH="%LIBRARY_PREFIX%" ^
 -DTIGL_VIEWER=ON ^
 -DTIGL_BINDINGS_MATLAB=ON ^
 -DPYTHON_EXECUTABLE:FILEPATH="%PYTHON%" ^
 -DTIGL_BUILD_TESTS=ON ^
 ..
if errorlevel 1 exit 1

REM Build step 
ninja
if errorlevel 1 exit 1

ninja doc
if errorlevel 1 exit 1

REM Install step
ninja install
if errorlevel 1 exit 1

REM Test step 
REM PATH="%LIBRARY_PREFIX%"\bin;%PATH% ninja check
REM if errorlevel 1 exit 1

set PATH=%PATH%;"%PREFIX%"\NSIS;"%PREFIX%"\NSIS\Bin
ninja package
if errorlevel 1 exit 1
copy *.zip "%RECIPE_DIR%"
copy *.exe "%RECIPE_DIR%"

REM install python packages
mkdir %SP_DIR%\tigl
echo. 2> %SP_DIR%\tigl\__init__.py
copy lib\tiglwrapper.py %SP_DIR%\tigl\

