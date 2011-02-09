@echo on

set SOLUTION_NAME="Src\Src.vcproj"

REM Build dll library version
devenv /clean Release_Dll /project TIGL %SOLUTION_NAME%
devenv /build Release_Dll /project TIGL %SOLUTION_NAME%

REM Build static library version
devenv /clean Release /project TIGL %SOLUTION_NAME%
devenv /build Release /project TIGL %SOLUTION_NAME%

REM Build TIGLViewer
devenv TIGLViewer\mfcsample\mfcsample.vcproj /project mfcsample /clean Release
devenv TIGLViewer\mfcsample\mfcsample.vcproj /project mfcsample /build Release

devenv TIGLViewer\TIGLViewer\TIGLViewer.vcproj /project TIGLViewer /clean Release
devenv TIGL.sln /project TIGLViewer /build Release

REM Build API-Docu
doxygen Doc/Doxyfile

rem - Build structure
rm -rf Install
mkdir Install\Static
mkdir Install\Dynamic
mkdir Install\Include
mkdir Install\TIGLViewer
mkdir Install\Doc

copy /Y Src\Release\TIGL.lib Install\Static
copy /Y Src\Release_Dll\TIGL.lib Install\Dynamic
copy /Y Src\Release_Dll\TIGL.dll Install\Dynamic
copy /Y Src\tigl.h Install\Include
copy /Y TIGLViewer\release Install\TIGLViewer
xcopy /E /Y doc\html Install\doc

REM - zip it 
7za a TIGL-0.X_Win32_msvc7.1_static_shared.zip Install
