find_package(OpenThreads)
find_package(osg)
find_package(osgUtil)
find_package(osgViewer)
find_package(osgGA)


set(PROGNAME "ReadVTK")

macro( config_project PROGNAME LIBNAME )
	include_directories( ${${LIBNAME}_INCLUDE_DIR})
	target_link_libraries( ${PROGNAME} ${${LIBNAME}_LIBRARY} )
endmacro()

include_directories(${TIXI_INCLUDE_DIRS})

set(SOURCE_FILES
	main.cpp
	MaterialTemplate.cpp
	MaterialTemplate.h
	GeometricVisObject.cpp 
	GeometricVisObject.h
	PickHandler.cpp
	PickHandler.h
	VirtualVisObject.cpp
	VirtualVisObject.h
        Visualizer.cpp
	Visualizer.h
	VisObject.cpp
	VisObject.h
	InputObject.cpp
	InputObject.h
        CrossNode.cpp
        CrossNode.h
        crossnodegeode.cpp
        crossnodegeode.h
)

add_executable(${PROGNAME} ${SOURCE_FILES})
target_link_libraries(${PROGNAME} 
	${TIXI_LIBRARIES} 
)

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
