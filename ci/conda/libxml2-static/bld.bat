cd win32


cscript configure.js iconv=no prefix="%LIBRARY_PREFIX%"
if errorlevel 1 exit 1

REM Build step 
nmake
if errorlevel 1 exit 1

REM Install step
nmake install
if errorlevel 1 exit 1

REM Remove test runners
del "%LIBRARY_PREFIX%"\bin\run*.exe /F /Q
del "%LIBRARY_PREFIX%"\bin\test*.exe /F /Q
if errorlevel 1 exit 1