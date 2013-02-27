find_package(OpenThreads)
find_package(osg)
find_package(osgUtil)
find_package(osgViewer)
find_package(osgGA)

set(PROGNAME "osgbasic")

macro( config_project PROGNAME LIBNAME )
	include_directories( ${${LIBNAME}_INCLUDE_DIR} )
	target_link_libraries( ${PROGNAME} ${${LIBNAME}_LIBRARY} )
endmacro()

add_executable(${PROGNAME} BasicGeometry.cpp)
config_project( ${PROGNAME} OPENTHREADS )
config_project( ${PROGNAME} OSG )
config_project( ${PROGNAME} OSGUTIL )
config_project( ${PROGNAME} OSGVIEWER )
config_project( ${PROGNAME} OSGGA )

# required opencascade libraries
set (TV_OCC_LIBS ${OCC_LIBS}
	PTKernel TKOffset TKOpenGl
	TKPShape TKSTEP
	TKService TKShapeSchema  
	TKV2d TKV3d TKVRML  
)

#find_package( OpenCASCADE REQUIRED COMPONENTS ${TV_OCC_LIBS})
