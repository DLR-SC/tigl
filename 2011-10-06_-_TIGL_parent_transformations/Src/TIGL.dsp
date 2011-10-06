# Microsoft Developer Studio Project File - Name="TIGL" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** NICHT BEARBEITEN **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=TIGL - Win32 Debug
!MESSAGE Dies ist kein gültiges Makefile. Zum Erstellen dieses Projekts mit NMAKE
!MESSAGE verwenden Sie den Befehl "Makefile exportieren" und führen Sie den Befehl
!MESSAGE 
!MESSAGE NMAKE /f "TIGL.mak".
!MESSAGE 
!MESSAGE Sie können beim Ausführen von NMAKE eine Konfiguration angeben
!MESSAGE durch Definieren des Makros CFG in der Befehlszeile. Zum Beispiel:
!MESSAGE 
!MESSAGE NMAKE /f "TIGL.mak" CFG="TIGL - Win32 Debug"
!MESSAGE 
!MESSAGE Für die Konfiguration stehen zur Auswahl:
!MESSAGE 
!MESSAGE "TIGL - Win32 Release" (basierend auf  "Win32 (x86) Static Library")
!MESSAGE "TIGL - Win32 Debug" (basierend auf  "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "TIGL - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "TIGL___Win32_Release"
# PROP BASE Intermediate_Dir "TIGL___Win32_Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "TIGL___Win32_Release"
# PROP Intermediate_Dir "temp"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GR /GX /O2 /I "../../local/include" /I "$(CASROOT)/inc" /D "NDEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /D "WNT" /D "IFORT_WIN32" /FD /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x407 /d "NDEBUG"
# ADD RSC /l 0x407 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "TIGL - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "TIGL___Win32_Debug"
# PROP BASE Intermediate_Dir "TIGL___Win32_Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "TIGL___Win32_Debug"
# PROP Intermediate_Dir "temp"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W2 /GR /GX /ZI /Od /I "../../local/include" /I "$(CASROOT)/inc" /D "_DEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /D "WNT" /D "IFORT_WIN32" /FD /GZ /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x407 /d "_DEBUG"
# ADD RSC /l 0x407 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ENDIF 

# Begin Target

# Name "TIGL - Win32 Release"
# Name "TIGL - Win32 Debug"
# Begin Group "Quellcodedateien"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\CCPACSConfiguration.cpp
# End Source File
# Begin Source File

SOURCE=.\CCPACSConfigurationManager.cpp
# End Source File
# Begin Source File

SOURCE=.\CCPACSFuselage.cpp
# End Source File
# Begin Source File

SOURCE=.\CCPACSFuselageConnection.cpp
# End Source File
# Begin Source File

SOURCE=.\CCPACSFuselagePositioning.cpp
# End Source File
# Begin Source File

SOURCE=.\CCPACSFuselagePositionings.cpp
# End Source File
# Begin Source File

SOURCE=.\CCPACSFuselageProfile.cpp
# End Source File
# Begin Source File

SOURCE=.\CCPACSFuselageProfiles.cpp
# End Source File
# Begin Source File

SOURCE=.\CCPACSFuselages.cpp
# End Source File
# Begin Source File

SOURCE=.\CCPACSFuselageSection.cpp
# End Source File
# Begin Source File

SOURCE=.\CCPACSFuselageSectionElement.cpp
# End Source File
# Begin Source File

SOURCE=.\CCPACSFuselageSectionElements.cpp
# End Source File
# Begin Source File

SOURCE=.\CCPACSFuselageSections.cpp
# End Source File
# Begin Source File

SOURCE=.\CCPACSFuselageSegment.cpp
# End Source File
# Begin Source File

SOURCE=.\CCPACSFuselageSegments.cpp
# End Source File
# Begin Source File

SOURCE=.\CCPACSHeader.cpp
# End Source File
# Begin Source File

SOURCE=.\CCPACSWing.cpp
# End Source File
# Begin Source File

SOURCE=.\CCPACSWingConnection.cpp
# End Source File
# Begin Source File

SOURCE=.\CCPACSWingPositioning.cpp
# End Source File
# Begin Source File

SOURCE=.\CCPACSWingPositionings.cpp
# End Source File
# Begin Source File

SOURCE=.\CCPACSWingProfile.cpp
# End Source File
# Begin Source File

SOURCE=.\CCPACSWingProfiles.cpp
# End Source File
# Begin Source File

SOURCE=.\CCPACSWings.cpp
# End Source File
# Begin Source File

SOURCE=.\CCPACSWingSection.cpp
# End Source File
# Begin Source File

SOURCE=.\CCPACSWingSectionElement.cpp
# End Source File
# Begin Source File

SOURCE=.\CCPACSWingSectionElements.cpp
# End Source File
# Begin Source File

SOURCE=.\CCPACSWingSections.cpp
# End Source File
# Begin Source File

SOURCE=.\CCPACSWingSegment.cpp
# End Source File
# Begin Source File

SOURCE=.\CCPACSWingSegments.cpp
# End Source File
# Begin Source File

SOURCE=.\CTiglAlgorithmManager.cpp
# End Source File
# Begin Source File

SOURCE=.\CTiglApproximateBsplineWire.cpp
# End Source File
# Begin Source File

SOURCE=.\CTiglError.cpp
# End Source File
# Begin Source File

SOURCE=.\CTiglInterpolateBsplineWire.cpp
# End Source File
# Begin Source File

SOURCE=.\CTiglInterpolateLinearWire.cpp
# End Source File
# Begin Source File

SOURCE=.\CTiglPoint.cpp
# End Source File
# Begin Source File

SOURCE=.\CTiglTransformation.cpp
# End Source File
# Begin Source File

SOURCE=.\tigl.cpp
# End Source File
# Begin Source File

SOURCE=.\tigl_fortran.cpp
# End Source File
# End Group
# Begin Group "Header-Dateien"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\CCPACSConfiguration.h
# End Source File
# Begin Source File

SOURCE=.\CCPACSConfigurationManager.h
# End Source File
# Begin Source File

SOURCE=.\CCPACSFuselage.h
# End Source File
# Begin Source File

SOURCE=.\CCPACSFuselageConnection.h
# End Source File
# Begin Source File

SOURCE=.\CCPACSFuselagePositioning.h
# End Source File
# Begin Source File

SOURCE=.\CCPACSFuselagePositionings.h
# End Source File
# Begin Source File

SOURCE=.\CCPACSFuselageProfile.h
# End Source File
# Begin Source File

SOURCE=.\CCPACSFuselageProfiles.h
# End Source File
# Begin Source File

SOURCE=.\CCPACSFuselages.h
# End Source File
# Begin Source File

SOURCE=.\CCPACSFuselageSection.h
# End Source File
# Begin Source File

SOURCE=.\CCPACSFuselageSectionElement.h
# End Source File
# Begin Source File

SOURCE=.\CCPACSFuselageSectionElements.h
# End Source File
# Begin Source File

SOURCE=.\CCPACSFuselageSections.h
# End Source File
# Begin Source File

SOURCE=.\CCPACSFuselageSegment.h
# End Source File
# Begin Source File

SOURCE=.\CCPACSFuselageSegments.h
# End Source File
# Begin Source File

SOURCE=.\CCPACSHeader.h
# End Source File
# Begin Source File

SOURCE=.\CCPACSWing.h
# End Source File
# Begin Source File

SOURCE=.\CCPACSWingConnection.h
# End Source File
# Begin Source File

SOURCE=.\CCPACSWingPositioning.h
# End Source File
# Begin Source File

SOURCE=.\CCPACSWingPositionings.h
# End Source File
# Begin Source File

SOURCE=.\CCPACSWingProfile.h
# End Source File
# Begin Source File

SOURCE=.\CCPACSWingProfiles.h
# End Source File
# Begin Source File

SOURCE=.\CCPACSWings.h
# End Source File
# Begin Source File

SOURCE=.\CCPACSWingSection.h
# End Source File
# Begin Source File

SOURCE=.\CCPACSWingSectionElement.h
# End Source File
# Begin Source File

SOURCE=.\CCPACSWingSectionElements.h
# End Source File
# Begin Source File

SOURCE=.\CCPACSWingSections.h
# End Source File
# Begin Source File

SOURCE=.\CCPACSWingSegment.h
# End Source File
# Begin Source File

SOURCE=.\CCPACSWingSegments.h
# End Source File
# Begin Source File

SOURCE=.\CTiglAlgorithmManager.h
# End Source File
# Begin Source File

SOURCE=.\CTiglApproximateBsplineWire.h
# End Source File
# Begin Source File

SOURCE=.\CTiglError.h
# End Source File
# Begin Source File

SOURCE=.\CTiglInterpolateBsplineWire.h
# End Source File
# Begin Source File

SOURCE=.\CTiglInterpolateLinearWire.h
# End Source File
# Begin Source File

SOURCE=.\CTiglPoint.h
# End Source File
# Begin Source File

SOURCE=.\CTiglTransformation.h
# End Source File
# Begin Source File

SOURCE=.\ITiglWireAlgorithm.h
# End Source File
# Begin Source File

SOURCE=.\tigl.h
# End Source File
# Begin Source File

SOURCE=.\tigl_fortran.h
# End Source File
# End Group
# End Target
# End Project
