# TIGLViewer cmake project
# author: Martin Siggel

# set up QT
find_package(Qt4)	

#build only tiglviewer if qt was found
if(QT4_FOUND)

include(${QT_USE_FILE})
add_definitions(${QT_DEFINITIONS})

find_package(OpenGL REQUIRED)

set(tv_src
	CommandLineParameters.cpp
	ISession_Point.cpp
	ISession_Text.cpp
	main.cpp
	TIGLViewerApplication.cpp
	TIGLViewerContext.cpp
	TIGLViewerDocument.cpp
	TIGLViewerInputoutput.cpp
	TIGLViewerInternal.h.cpp
	TIGLViewerWidget.cpp
	TIGLViewerWindow.cpp
	)
	
set(tv_hdr
	CommandLineParameters.h
	ISession_Point.h
	ISession_Text.h
	TIGLDebugStream.h
	TIGLViewerApplication.h
	TIGLViewerContext.h
	TIGLViewerDocument.h
	TIGLViewerInputoutput.h
	TIGLViewer.h
	TIGLViewerInputoutput.h
	TIGLViewerWidget.h
	TIGLViewerWindow.h
)

set(tv_ui_comp
	src/TIGLViewerWindow.ui
)
	
foreach ( _fname ${tv_src})
   set(tv_src_comp ${tv_src_comp} src/${_fname})
endforeach()
foreach ( _fname ${tv_hdr})
   set(tv_hdr_comp ${tv_hdr_comp} src/${_fname})
endforeach()

# moc header files
QT4_WRAP_CPP(tv_hdr_MOC ${tv_hdr_comp})
QT4_WRAP_UI(tv_ui_HEADERS ${tv_ui_comp})

add_definitions(-DCSFDB -DQOCC_STATIC)
include_directories( ${OpenCASCADE_INCLUDE_DIR} ${TIXI_INCLUDE_DIRS})
include_directories( ${PROJECT_SOURCE_DIR}/src ${CMAKE_CURRENT_SOURCE_DIR}/src) 

# include path for ui files
include_directories( ${CMAKE_CURRENT_BINARY_DIR} )

source_group(MOCS REGULAR_EXPRESSION "moc_")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)

add_executable(TIGLViewer 
		${tv_src_comp} 
		${tv_hdr_MOC} 
		${tv_hdr_comp} 
		${tv_ui_comp}
)

target_link_libraries(TIGLViewer 
	${QT_LIBRARIES} 
	${OPENGL_LIBRARIES} 
	${OpenCASCADE_LIBRARIES} 
	${TIXI_LIBRARIES} 
	TIGL_static
)

else(QT4_FOUND)
message(STATUS "No qt4 was found. TIGLViewer will not be build!")
endif(QT4_FOUND)
