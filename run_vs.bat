@echo off
REM ==============================================================
REM  TiGL Windows Build + Launch Visual Studio 2022
REM  Assumes you run this from the "Developer Command Prompt for VS 2022"
REM ==============================================================

setlocal enabledelayedexpansion

REM --- Project configuration ------------------------------------
set "PIXI_ENV=python-internal"
set "BUILD_DIR=build-vs"
set "GENERATOR=Visual Studio 17 2022"
set "PLATFORM=x64"
set "INSTALL_DIR=%CD%\%BUILD_DIR%\install"
set "CMAKE_BUILD_TYPE=Debug"
REM ---------------------------------------------------------------

REM --- Optional arguments ---------------------------------------
set "FORCE_RECONFIGURE=0"
if /I "%~1"=="--reconfigure" set "FORCE_RECONFIGURE=1"
if /I "%~1"=="--clean" (
  echo [CLEAN] Removing "%BUILD_DIR%"...
  rmdir /S /Q "%BUILD_DIR%" 2>nul
  set "FORCE_RECONFIGURE=1"
)
REM ---------------------------------------------------------------

if not exist "%BUILD_DIR%" mkdir "%BUILD_DIR%"

set "NEED_CONFIGURE=0"
if not exist "%BUILD_DIR%\TIGL.sln" set "NEED_CONFIGURE=1"
if not exist "%BUILD_DIR%\CMakeCache.txt" set "NEED_CONFIGURE=1"
if %FORCE_RECONFIGURE%==1 set "NEED_CONFIGURE=1"

if %NEED_CONFIGURE%==1 (
  echo [CONFIGURE] Running CMake inside pixi environment...
  pixi run -e %PIXI_ENV% -- ^
    cmake -S . -B "%BUILD_DIR%" ^
    -G "%GENERATOR%" -A %PLATFORM% ^
    -D CMAKE_INSTALL_PREFIX=%INSTALL_DIR% ^
    -D TIGL_BINDINGS_PYTHON_INTERNAL=ON ^
    -D Python3_FIND_STRATEGY=LOCATION ^
    -D Python3_FIND_FRAMEWORK=NEVER ^
    -D TIGL_BUILD_TESTS=ON ^
    -D TIGL_BINDINGS_PYTHON=ON ^
    -D TIGL_BINDINGS_MATLAB=OFF ^
    -D TIGL_CREATOR=ON ^
	-D TIGL_CONCAT_GENERATED_FILES=OFF ^
    -D OCE_STATIC_LIBS=OFF
) else (
  echo [CONFIGURE] Existing CMake config detected. Skipping.
)

if not exist "%BUILD_DIR%\TIGL.sln" (
  echo ERROR: Solution "%BUILD_DIR%\TIGL.sln" not found.
  exit /b 1
)

echo [VS] Launching Visual Studio...
pixi run -e %PIXI_ENV% -- devenv "%CD%\%BUILD_DIR%\TIGL.sln"

endlocal
