# Microsoft Developer Studio Project File - Name="CUnitTests" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** NICHT BEARBEITEN **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=CUnitTests - Win32 Debug
!MESSAGE Dies ist kein gültiges Makefile. Zum Erstellen dieses Projekts mit NMAKE
!MESSAGE verwenden Sie den Befehl "Makefile exportieren" und führen Sie den Befehl
!MESSAGE 
!MESSAGE NMAKE /f "CUnitTests.mak".
!MESSAGE 
!MESSAGE Sie können beim Ausführen von NMAKE eine Konfiguration angeben
!MESSAGE durch Definieren des Makros CFG in der Befehlszeile. Zum Beispiel:
!MESSAGE 
!MESSAGE NMAKE /f "CUnitTests.mak" CFG="CUnitTests - Win32 Debug"
!MESSAGE 
!MESSAGE Für die Konfiguration stehen zur Auswahl:
!MESSAGE 
!MESSAGE "CUnitTests - Win32 Release" (basierend auf  "Win32 (x86) Console Application")
!MESSAGE "CUnitTests - Win32 Debug" (basierend auf  "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "CUnitTests - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "CUnitTests___Win32_Release"
# PROP BASE Intermediate_Dir "CUnitTests___Win32_Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "CUnitTests___Win32_Release"
# PROP Intermediate_Dir "temp"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /MD /Za /W3 /O2 /I "." /I "./TestFunctions" /I "../Src" /I "../../local/include" /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /FD /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x407 /d "NDEBUG"
# ADD RSC /l 0x407 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 TIXI.lib libxml2.lib libcunit_vc_d.lib TKVrml.lib TKStl.lib TKBRep.lib TKIGES.lib TKShHealing.lib TKSTEP.lib TKXSBase.lib TKShapeSchema.lib FWOSPlugin.lib PTKernel.lib TKBool.lib TKCAF.lib TKCDF.lib TKDraw.lib TKernel.lib TKFeat.lib TKFillet.lib TKG2d.lib TKG3d.lib TKGeomAlgo.lib TKGeomBase.lib TKHLR.lib TKMath.lib TKOffset.lib TKPCAF.lib TKPrim.lib TKPShape.lib TKService.lib TKTopAlgo.lib TKV2d.lib TKV3d.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386 /nodefaultlib:"MSVCRTD" /libpath:"../../local/lib" /libpath:"$(CASROOT)/win32/lib"
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "CUnitTests - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "CUnitTests___Win32_Debug"
# PROP BASE Intermediate_Dir "CUnitTests___Win32_Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "CUnitTests___Win32_Debug"
# PROP Intermediate_Dir "temp"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /Za /W3 /ZI /Od /I "." /I "./TestFunctions" /I "../Src" /I "../../local/include" /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /FD /GZ /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x407 /d "_DEBUG"
# ADD RSC /l 0x407 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 TIXI.lib libxml2.lib libcunit_vc_d.lib TKVrml.lib TKStl.lib TKBRep.lib TKIGES.lib TKShHealing.lib TKSTEP.lib TKXSBase.lib TKShapeSchema.lib FWOSPlugin.lib PTKernel.lib TKBool.lib TKCAF.lib TKCDF.lib TKDraw.lib TKernel.lib TKFeat.lib TKFillet.lib TKG2d.lib TKG3d.lib TKGeomAlgo.lib TKGeomBase.lib TKHLR.lib TKMath.lib TKOffset.lib TKPCAF.lib TKPrim.lib TKPShape.lib TKService.lib TKTopAlgo.lib TKV2d.lib TKV3d.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /nodefaultlib:"libc" /nodefaultlib:"MSVCRT" /pdbtype:sept /libpath:"../../local/lib" /libpath:"$(CASROOT)/win32/lib" /libpath:""$(CASROOT)/win32/lib""

!ENDIF 

# Begin Target

# Name "CUnitTests - Win32 Release"
# Name "CUnitTests - Win32 Debug"
# Begin Group "Quellcodedateien"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Group "TestFunctions"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\TestFunctions\cpacsConfiguration.c
# End Source File
# Begin Source File

SOURCE=.\TestFunctions\tiglAlgorithm.c
# End Source File
# Begin Source File

SOURCE=.\TestFunctions\tiglFuselageGetPoint.c
# End Source File
# Begin Source File

SOURCE=.\TestFunctions\tiglFuselageSegment.c
# End Source File
# Begin Source File

SOURCE=.\TestFunctions\tiglWing.c
# End Source File
# Begin Source File

SOURCE=.\TestFunctions\tiglWingGetPoint.c
# End Source File
# Begin Source File

SOURCE=.\TestFunctions\tiglWingSegment.c
# End Source File
# End Group
# Begin Source File

SOURCE=.\addTestFunctions.c
# End Source File
# Begin Source File

SOURCE=.\CUMain.c
# End Source File
# End Group
# Begin Group "Header-Dateien"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\TestFunctions\testFunctions.h
# End Source File
# End Group
# Begin Group "Ressourcendateien"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
