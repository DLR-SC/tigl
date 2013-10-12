# TIGLViewer cmake project
# author: Martin Siggel

# set up QT

OPTION(TIGL_VIEWER "Builds the TiGL Viewer program (requires OpenGL and Qt4)" ON)

set(PROGNAME "TIGLViewer")


if(TIGL_VIEWER)

find_package(Qt4)
#build only tiglviewer if qt was found
if(QT4_FOUND)	

# required opencascade libraries
set (TV_OCC_LIBS 
	TKOpenGl 
	TKVRML TKShapeSchema TKPShape TKCDF PTKernel ${OCC_VISU_LIBS} ${OCC_LIBS}
)

set(OpenCASCADE_LIBRARIES "")
find_package( OpenCASCADE REQUIRED COMPONENTS ${TV_OCC_LIBS})


# we need opengl in order to build tiglviewer
find_package(OpenGL REQUIRED)

set( QT_USE_QTSCRIPT TRUE )
include(${QT_USE_FILE} ${GLOG_INCLUDE_DIR})
add_definitions(${QT_DEFINITIONS})

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
    TIGLViewerSettingsDialog.cpp
    TIGLScriptProxy.cpp
    TIXIScriptProxy.cpp
    TIGLScriptEngine.cpp
    TiglViewerConsole.cpp
    TIGLViewerSettings.cpp
    TIGLViewerControlFile.cpp
    TIGLViewerEtaXsiDialog.cpp
    CHotsoseMeshReader.cpp
)
	
# normal header files
set(tv_hdr
    CommandLineParameters.h
    ISession_Point.h
    ISession_Text.h
    TIGLViewer.h
    TIGLViewerSettings.h
    TIGLViewerControlFile.h
    CHotsoseMeshReader.h
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
	TIGLViewerSettingsDialog.h
	TIGLScriptProxy.h
	TIXIScriptProxy.h
	TIGLScriptEngine.h
	TiglViewerConsole.h
	TIGLViewerEtaXsiDialog.h
)

set(tv_ui_comp
	src/TIGLViewerWindow.ui
	src/TIGLViewerSettingsDialog.ui
	src/TIGLViewerEtaXsiDialog.ui
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

include_directories( ${OpenCASCADE_INCLUDE_DIR} ${TIXI_INCLUDE_DIRS})
include_directories( ${PROJECT_SOURCE_DIR}/src ${CMAKE_CURRENT_SOURCE_DIR}/src ${PROJECT_BINARY_DIR}/src)

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

if(APPLE)
   set(TYPE MACOSX_BUNDLE)

   set( MACOSX_BUNDLE_SHORT_VERSION_STRING ${TIGL_VERSION} )
   set( MACOSX_BUNDLE_VERSION ${TIGL_VERSION} )
   set( MACOSX_BUNDLE_LONG_VERSION_STRING Version ${TIGL_VERSION} )

   # add program icon
   set(MACOSX_BUNDLE_ICON_FILE ${PROGNAME}.icns )
endif()

add_executable(${PROGNAME} ${TYPE}
		${tv_src_comp} 
		${tv_hdr_MOC} 
 		${tv_hdr_qobj_comp}        
		${tv_hdr_comp} 
		${tv_ui_comp}
		${tv_rcrs}
		${tv_icon_rc}
)

target_link_libraries(${PROGNAME} 
	TIGL_static
	${TIXI_LIBRARIES} 
	${QT_LIBRARIES} 
	${OPENGL_LIBRARIES} 
	${OpenCASCADE_LIBRARIES} 
)

if(GLOG_FOUND)
  include_directories(${GLOG_INCLUDE_DIR})
  target_link_libraries (${PROGNAME} ${GLOG_LIBRARIES})
endif(GLOG_FOUND)

#--------------------------------------------------------------------------------
# Now the installation stuff below
#--------------------------------------------------------------------------------

#--------------------------------------------------------------------------------
# Install the TIGLViewer application, on Apple, the bundle is at the root of the
# install tree, and on other platforms it'll go into the bin directory.
INSTALL(TARGETS ${PROGNAME} 
    BUNDLE DESTINATION . COMPONENT viewer
    RUNTIME DESTINATION bin COMPONENT viewer
    )


set(APPS "bin/${PROGNAME}")
IF(APPLE)
  set(APPS "${PROGNAME}.app")

  # TKOpenGL is loaded as a plugin, hence we must manually install it
  INSTALL(FILES ${TKOpenGl_OCCLIB} DESTINATION ${PROGNAME}.app/Contents/MacOS/ COMPONENT viewer)
  # copy icon
  INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/gfx/${PROGNAME}.icns DESTINATION ${PROGNAME}.app/Contents/Resources/ COMPONENT viewer)
ENDIF(APPLE)
IF(WIN32)
  set(APPS "bin/${PROGNAME}.exe")

  # install runtime libs
  FILE(TO_CMAKE_PATH "$ENV{SYSTEMROOT}" SYSTEMROOT)
  if(OCC_VERSION_STRING VERSION_EQUAL "6.3")
    SET(CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS
      "${SYSTEMROOT}/system32/msvcp71.dll"
      "${SYSTEMROOT}/system32/msvcr71.dll"
    )
  endif()

  # TKOpenGL is loaded as a plugin, hence we must manually install it
  set( OCC_PLUGINS TKOpenGl.dll FWOSPlugin.dll)
  foreach( _plug ${OCC_PLUGINS} )
     set(OCC_PLUGINS_FULL ${OCC_PLUGINS_FULL} ${OpenCASCADE_DLL_DIRECTORY}/${_plug} )
  endforeach()
  INSTALL(FILES ${OCC_PLUGINS_FULL} DESTINATION bin COMPONENT viewer)
ENDIF(WIN32)

INCLUDE(InstallRequiredSystemLibraries)

	
# set directories to look for dependencies
set(DIRS ${TIXI_BINARY_DIR} ${OpenCASCADE_DLL_DIRECTORY}  ${OpenCASCADE_LINK_DIRECTORY} ${QT_BINARY_DIR} ${TIGL_ADD_LIB_PATH})

if (WIN32 OR APPLE)
  include(DeployQt4)
  INSTALL_QT4_EXECUTABLE(${APPS} "" "${OCC_PLUGINS_FULL}" "${DIRS}" "" "" viewer)
endif()

else (QT4_FOUND)
message(STATUS "No qt4 was found. TIGLViewer will not be build!")
endif(QT4_FOUND)

endif(TIGL_VIEWER)


