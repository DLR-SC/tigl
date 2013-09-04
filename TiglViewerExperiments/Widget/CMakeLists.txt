find_package(OpenThreads)
find_package(osg)
find_package(osgUtil)
find_package(osgViewer)
find_package(osgGA)
find_package(osgQt)
find_package(osgDB)


set(PROGNAME "Widget")

FIND_PACKAGE(Boost)

FIND_PACKAGE(Qt4 4.4.3 REQUIRED QtCore QtGui QtOpengl)
include(${QT_USE_FILE})
add_definitions(${QT_DEFINITIONS})



macro( config_project PROGNAME LIBNAME )
	include_directories( ${${LIBNAME}_INCLUDE_DIR} ${Boost_INCLUDE_DIR})
	target_link_libraries( ${PROGNAME} ${${LIBNAME}_LIBRARY} )
endmacro()

include_directories(${PROJECT_SOURCE_DIR}/src ${OpenCASCADE_INCLUDE_DIR}  ${GLOG_INCLUDE_DIR} ${TIXI_INCLUDE_DIRS} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${PROJECT_BINARY_DIR}/src)

set(SOURCE_FILES
	RenderingThread.h
	AuxiliaryViewUpdater.h
	AuxiliaryViewUpdater.cpp
	crossnodegeode.h
	crossnodegeode.cpp
	CrossNode.h
	CrossNode.cpp
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
	ViewInteractionsHandler.h
	ViewInteractionsHandler.cpp
	SelectionBoxHandler.cpp
	SelectionBoxHandler.h
)

set(HEADERS_QOBJ VisualizerWidget.h MainWindow.h)
QT4_WRAP_CPP(HEADER_MOC ${HEADERS_QOBJ})
QT4_WRAP_UI(HEADER_UI MainWindow.ui)

add_executable(${PROGNAME} ${SOURCE_FILES} ${HEADER_MOC} ${HEADER_UI})
target_link_libraries(${PROGNAME} 
	TIGL_static
	${OpenCASCADE_LIBRARIES}
	${TIXI_LIBRARIES} 
	${GLOG_LIBRARIES}
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
