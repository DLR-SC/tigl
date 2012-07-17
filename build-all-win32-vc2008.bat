@echo on

set SOLUTION_NAME="Src\Src.vcproj"

REM Build dll library version
devenv /clean Release_Dll /project TIGL %SOLUTION_NAME%
devenv /build Release_Dll /project TIGL %SOLUTION_NAME%

REM Build static library version
devenv /clean Release /project TIGL %SOLUTION_NAME%
devenv /build Release /project TIGL %SOLUTION_NAME%

REM Build TIGLViewer
cd TIGLViewer
nmake clean
qmake
nmake release
cd ..

REM Build API-Docu
doxygen Doc/Doxyfile

rem - Build directory structure
rm -rf Install
mkdir Install\TIGL\Static
mkdir Install\TIGL\Dynamic
mkdir Install\TIGL\Include
mkdir Install\TIGL\Doc
mkdir Install\TIGLViewer

copy /Y Src\Release\TIGL.lib Install\TIGL\Static
copy /Y Src\Release_Dll\TIGL.lib Install\TIGL\Dynamic
copy /Y Src\Release_Dll\TIGL.dll Install\TIGL\Dynamic
copy /Y Src\tigl.h Install\TIGL\Include
xcopy /E /Y doc\html Install\TIGL\doc
copy /Y TIGLViewer\release\TIGLViewer.exe Install\TIGLViewer
copy /Y TIGLViewer\release\TIGLViewer.lib Install\TIGLViewer

REM - zip it 
cd Install
7za a ..\TIGL-0.X_Win32_msvc2008_static_shared.zip TIGL
7za a ..\TIGLViewer-0.X_Win32.zip TIGLViewer
cd..

