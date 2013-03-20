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

include_directories(${PROJECT_SOURCE_DIR}/src ${OpenCASCADE_INCLUDE_DIR}  ${GLOG_INCLUDE_DIR} ${TIXI_INCLUDE_DIRS} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})

set(SOURCE_FILES
	CrossNodeCallback.h
	VisualizerWidget.cpp
	VisualizerWidget.h
	InputObject.h
	InputObject.cpp
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
	SelectionBoxHandler.cpp
	SelectionBoxHandler.h
)

set(HEADERS_QOBJ VisualizerWidget.h MainWindow.h)
QT4_WRAP_CPP(HEADER_MOC ${HEADERS_QOBJ})
QT4_WRAP_UI(HEADER_UI MainWindow.ui)

add_executable(${PROGNAME} ${SOURCE_FILES} ${HEADER_MOC} ${HEADER_UI})
target_link_libraries(${PROGNAME} 
	${TIXI_LIBRARIES} 
	${OpenCASCADE_LIBRARIES} 
	${GLOG_LIBRARIES}
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
