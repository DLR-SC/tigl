find_package(OpenThreads)
find_package(osg)
find_package(osgUtil)
find_package(osgViewer)
find_package(osgGA)
find_package(osgQt)
find_package(osgDB)


find_package(Boost)

set(PROGNAME "Widget")

FIND_PACKAGE(Qt)
INCLUDE_DIRECTORIES(${QT_INCLUDE_DIR})



macro( config_project PROGNAME LIBNAME )
	include_directories( ${${LIBNAME}_INCLUDE_DIR} ${Boost_INCLUDE_DIR})
	target_link_libraries( ${PROGNAME} ${${LIBNAME}_LIBRARY} )
endmacro()

include_directories(${TIXI_INCLUDE_DIRS} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})

set(SOURCE_FILES
	VisualizerWidget.cpp
	VisualizerWidget.h
	MainWindow.h
	MainWindow.cpp 
	main.cpp
	MaterialTemplate.cpp
	MaterialTemplate.h
	GeometricVisObject.cpp 
	GeometricVisObject.h
	PickHandler.cpp
	PickHandler.h
	VirtualVisObject.cpp
	VirtualVisObject.h
	VisObject.h
	InputObject.cpp
	InputObject.h
	SelectionBoxHandler.cpp
	SelectionBoxHandler.h
)

set(HEADERS_QOBJ VisualizerWidget.h MainWindow.h)
QT4_WRAP_CPP(HEADER_MOC ${HEADERS_QOBJ})
QT4_WRAP_UI(HEADER_UI MainWindow.ui)

add_executable(${PROGNAME} ${SOURCE_FILES} ${HEADER_MOC} ${HEADER_UI})
target_link_libraries(${PROGNAME} 
	${TIXI_LIBRARIES} 
	${OCC_LIBS}
	TIGL_static
)

TARGET_LINK_LIBRARIES(${PROGNAME} ${QT_QTCORE_LIBRARY}
	${QT_QTGUI_LIBRARY})

config_project( ${PROGNAME} OPENTHREADS )
config_project( ${PROGNAME} OSG )
config_project( ${PROGNAME} OSGUTIL )
config_project( ${PROGNAME} OSGVIEWER )
config_project( ${PROGNAME} OSGGA )
config_project( ${PROGNAME} OSGQT )
config_project( ${PROGNAME} OSGDB )

# required opencascade libraries
set (TV_OCC_LIBS ${OCC_LIBS}
	PTKernel TKOffset TKOpenGl
	TKPShape TKSTEP
	TKService TKShapeSchema  
	TKV2d TKV3d TKVRML  
)

#find_package( OpenCASCADE REQUIRED COMPONENTS ${TV_OCC_LIBS})
