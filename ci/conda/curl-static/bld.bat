cd winbuild

nmake /f Makefile.vc mode=static DEBUG=no ENABLE_IDN=no
if errorlevel 1 exit 1

REM Install step
if %ARCH%==32 ( 
  xcopy /e ..\builds\libcurl-vc-x86-release-static-ipv6-sspi-winssl "%LIBRARY_PREFIX%"
) else (
  xcopy /e ..\builds\libcurl-vc-x64-release-static-ipv6-sspi-winssl "%LIBRARY_PREFIX%"
)
if errorlevel 1 exit 1