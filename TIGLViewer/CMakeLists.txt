# TIGLViewer cmake project
# author: Martin Siggel

# set up QT
find_package(Qt4)	

#build only tiglviewer if qt was found
if(QT4_FOUND)

SET( QT_USE_QTSCRIPT TRUE )
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
    TIGLScriptProxy.cpp
    TIXIScriptProxy.cpp
    TIGLScriptEngine.cpp
    TiglViewerConsole.cpp
)
	
# normal header files
set(tv_hdr
    CommandLineParameters.h
    ISession_Point.h
    ISession_Text.h
    TIGLViewer.h
)

# qobject header files that have to be moced
set(tv_hdr_qobj
	TIGLDebugStream.h
	TIGLViewerApplication.h
	TIGLViewerContext.h
	TIGLViewerDocument.h
	TIGLViewerInputoutput.h
	TIGLViewerInputoutput.h
	TIGLViewerWidget.h
	TIGLViewerWindow.h
	TIGLScriptProxy.h
	TIXIScriptProxy.h
	TIGLScriptEngine.h
	TiglViewerConsole.h
)

set(tv_ui_comp
	src/TIGLViewerWindow.ui
)
	
foreach ( _fname ${tv_src})
   set(tv_src_comp ${tv_src_comp} src/${_fname})
endforeach()
foreach ( _fname ${tv_hdr_qobj})
   set(tv_hdr_qobj_comp ${tv_hdr_qobj_comp} src/${_fname})
endforeach()
foreach ( _fname ${tv_hdr})
   set(tv_hdr_comp ${tv_hdr_comp} src/${_fname})
endforeach()

# moc header files
QT4_WRAP_CPP(tv_hdr_MOC ${tv_hdr_qobj_comp})
QT4_WRAP_UI(tv_ui_HEADERS ${tv_ui_comp})
# compile icons
QT4_ADD_RESOURCES( tv_rcrs src/TIGLViewer.qrc )

add_definitions(-DCSFDB -DQOCC_STATIC)

if(OCC_VERSION_STRING VERSION_GREATER "6.3")
    add_definitions(-DOCC_PATCHED)
endif()

include_directories( ${OpenCASCADE_INCLUDE_DIR} ${TIXI_INCLUDE_DIRS})
include_directories( ${PROJECT_SOURCE_DIR}/src ${CMAKE_CURRENT_SOURCE_DIR}/src )

# include path for ui files
include_directories( ${CMAKE_CURRENT_BINARY_DIR} )

source_group(MOCS REGULAR_EXPRESSION "moc_")
source_group(RESOURCES REGULAR_EXPRESSION "qrc_")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)

if(WIN32)
  # comment this line if you want to see an additional console window
  #set(TYPE WIN32)
  # add program icon
  set(tv_icon_rc src/TIGLViewer.rc)
endif(WIN32)

add_executable(TIGLViewer ${TYPE}
		${tv_src_comp} 
		${tv_hdr_MOC} 
 		${tv_hdr_qobj_comp}        
		${tv_hdr_comp} 
		${tv_ui_comp}
        ${tv_rcrs}
		${tv_icon_rc}
)

target_link_libraries(TIGLViewer 
	TIGL_static
	${TIXI_LIBRARIES} 
	${QT_LIBRARIES} 
	${OPENGL_LIBRARIES} 
	${OpenCASCADE_LIBRARIES} 
)

#--------------------------------------------------------------------------------
# Now the installation stuff below
#--------------------------------------------------------------------------------
# install runtime libs
if(WIN32)
  FILE(TO_CMAKE_PATH "$ENV{SYSTEMROOT}" SYSTEMROOT)
  if(OCC_VERSION_STRING VERSION_EQUAL "6.3")
    SET(CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS
      "${SYSTEMROOT}/system32/msvcp71.dll"
      "${SYSTEMROOT}/system32/msvcr71.dll"
    )
  endif()
endif(WIN32)
INCLUDE(InstallRequiredSystemLibraries)


SET(APPS "\${CMAKE_INSTALL_PREFIX}/bin/TIGLViewer")
IF(APPLE)
  SET(APPS "\${CMAKE_INSTALL_PREFIX}/TIGLViewer.app")
ENDIF(APPLE)
IF(WIN32)
  SET(APPS "\${CMAKE_INSTALL_PREFIX}/bin/TIGLViewer.exe")
ENDIF(WIN32)

#--------------------------------------------------------------------------------
# Install the TIGLViewer application, on Apple, the bundle is at the root of the
# install tree, and on other platforms it'll go into the bin directory.
INSTALL(TARGETS TIGLViewer 
    BUNDLE DESTINATION . COMPONENT viewer
    RUNTIME DESTINATION bin COMPONENT viewer
    )

# TKOpenGL is loaded as a plugin, hence we must manually install it
if(WIN32)
  set( OCC_PLUGINS TKOpenGl.dll )
  foreach( _plug ${OCC_PLUGINS} )
     set(OCC_PLUGINS_FULL ${OCC_PLUGINS_FULL} ${OpenCASCADE_DLL_DIRECTORY}/${_plug} )
  endforeach()
  INSTALL(FILES ${OCC_PLUGINS_FULL} DESTINATION bin COMPONENT viewer)
endif(WIN32)	
	
# directories to look for dependencies
SET(DIRS ${TIXI_BINARY_DIR} ${OpenCASCADE_DLL_DIRECTORY} ${QT_BINARY_DIR} ${ADD_LIB_PATH})

if (WIN32 OR APPLE)

# Now the work of copying dependencies into the bundle/package
# The quotes are escaped and variables to use at install time have their $ escaped
# An alternative is the do a configure_file() on a script and use install(SCRIPT  ...).
INSTALL(CODE "
    include(BundleUtilities)
    fixup_bundle(\"${APPS}\" \"${OCC_PLUGINS_FULL}\" \"${DIRS}\")
    " COMPONENT viewer)

endif()

else(QT4_FOUND)
message(STATUS "No qt4 was found. TIGLViewer will not be build!")
endif(QT4_FOUND)
