# TIGLViewer cmake project
# author: Martin Siggel

# set up QT

OPTION(TIGL_VIEWER "Builds the TiGL Viewer program (requires OpenGL and Qt4)" ON)


if (APPLE)
   set(PROGNAME "TiGL-Viewer3")
else(APPLE)
   set(PROGNAME "tiglviewer-3")
endif(APPLE)


if(TIGL_VIEWER)

include(SearchQt)

#build only tiglviewer if qt was found
if(QT4_FOUND OR Qt5Widgets_FOUND)

include(tiglmacros)
AddToCheckstyle()

# required opencascade libraries
set (TV_OCC_LIBS 
    TKOpenGl 
    ${OCC_VISU_LIBS} ${OCC_LIBS}
)

if(NOT OpenCASCADE_DONT_SEARCH_OCE)
  find_package(OCE 0.15 COMPONENTS ${TV_OCC_LIBS} QUIET)
endif()

if(OCE_FOUND)
  set(OpenCASCADE_LIBRARIES ${TV_OCC_LIBS})
  set(OpenCASCADE_INCLUDE_DIR ${OCE_INCLUDE_DIRS})
else(OCE_FOUND)
  set(OpenCASCADE_LIBRARIES "")
  find_package(OpenCASCADE REQUIRED COMPONENTS ${TV_OCC_LIBS})
endif(OCE_FOUND)

# we need opengl in order to build tiglviewer
find_package(OpenGL REQUIRED)

if(CMAKE_SYSTEM_NAME STREQUAL Linux)
  # set library search path to local lib directory during installation
  set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE) 
  set(RELDIR "../${CMAKE_INSTALL_LIBDIR}")

  if (IS_ABSOLUTE ${CMAKE_INSTALL_LIBDIR})
    # get relative path to tiglviewer binary
    file(RELATIVE_PATH RELDIR ${CMAKE_INSTALL_PREFIX}/bin "${CMAKE_INSTALL_LIBDIR}")
  endif()
  
  set(CMAKE_INSTALL_RPATH \$ORIGIN/${RELDIR})

  if (NOT RELDIR STREQUAL "../lib")
    message(STATUS ${RELDIR})
    set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_RPATH}:\$ORIGIN/../lib")
  endif()
endif() # linux‚

# add all tigl directories to include path
set(TIGL_INCLUDES ${PROJECT_SOURCE_DIR}/src/)
subdirlist(SUBDIRS ${PROJECT_SOURCE_DIR}/src/)
foreach(subdir ${SUBDIRS})
  set(TIGL_INCLUDES ${TIGL_INCLUDES} ${PROJECT_SOURCE_DIR}/src/${subdir})
endforeach()

include_directories(${TIGL_INCLUDES})

set(tv_src
    ISession_Point.cpp
    ISession_Direction.cpp
    ISession_Text.cpp
    main.cpp
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
    TIGLAISTriangulation.cpp
    TIGLViewerErrorDialog.cpp
    TIGLViewerLogHistory.cpp
    TIGLViewerLogRedirection.cpp
    TIGLViewerLoggerHTMLDecorator.cpp
    TIGLViewerFuseDialog.cpp
    TIGLViewerShapeIntersectionDialog.cpp
    TIGLViewerScreenshotDialog.cpp
    TIGLViewerDrawVectorDialog.cpp
    TIGLViewerScopedCommand.cpp
    TIGLQAspectWindow.cpp
    TIGLViewerVTKExportDialog.cpp
    TIGLSliderDialog.cpp
    TIGLViewerApp.cpp
    TIGLGeometryChoserDialog.cpp
    TIGLViewerUndoCommands.cpp
    # files for CPACSCreator
	modificators/ModificatorWidget.cpp
	modificators/ModificatorElementWidget.cpp
	modificators/ModificatorSectionWidget.cpp
	modificators/ModificatorFuselageWidget.cpp
	modificators/ModificatorTransformationWidget.cpp
	modificators/ModificatorWingWidget.cpp
	modificators/CTiglPointWidget.cpp
	modificators/ModificatorFuselagesWidget.cpp
	modificators/NewFuselageDialog.cpp
	modificators/NewConnectedElementDialog.cpp
	modificators/SymmetryComboBoxWidget.cpp
	modificators/ModificatorSectionsWidget.cpp
	modificators/ModificatorWingsWidget.cpp
	modificators/NewWingDialog.cpp
	ModificatorManager.cpp
	CPACSAbstractModel.cpp
	CPACSTreeItem.cpp
	CPACSTree.cpp
	XPathParser.cpp
	TIGLViewerException.cpp
	ModificatorContainerWidget.cpp
	CPACSTreeWidget.cpp
	CPACSFilterModel.cpp
	TIGLViewerNewFileDialog.cpp
	TIGLViewerCommonFunctions.cpp
)
	
# normal header files
set(tv_hdr
    CommandLineParameters.h
    ISession_Point.h
    ISession_Direction.h
    ISession_Text.h
    TIGLViewer.h
    TIGLViewerSettings.h
    TIGLViewerControlFile.h
    TIGLAISTriangulation.h
    CHotsoseMeshReader.h
    TIGLViewerLogHistory.h
    TIGLViewerLoggerHTMLDecorator.h
    TIGLViewerScopedCommand.h
    TIGLQAspectWindow.h
    TIGLViewerUndoCommands.h
    # files for CPACSCreator
	CPACSTreeItem.h
	CPACSTree.h
	XPathParser.h
	TIGLViewerException.h
	TIGLViewerCommonFunctions.h
)

# qobject header files that have to be moced
set(tv_hdr_qobj
	TIGLDebugStream.h
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
	TIGLViewerErrorDialog.h
	TIGLViewerDrawVectorDialog.h
	TIGLViewerLogRedirection.h
	TIGLViewerFuseDialog.h
	TIGLViewerShapeIntersectionDialog.h
	TIGLViewerScreenshotDialog.h
	TIGLViewerVTKExportDialog.h
	TIGLSliderDialog.h
	TIGLViewerApp.h
	TIGLGeometryChoserDialog.h
	# files for CPACSCreator
	modificators/ModificatorWidget.h
	modificators/CTiglPointWidget.h
	modificators/ModificatorElementWidget.h
	modificators/ModificatorSectionWidget.h
	modificators/ModificatorFuselageWidget.h
	modificators/ModificatorTransformationWidget.h
	modificators/ModificatorWingWidget.h
	modificators/ModificatorFuselagesWidget.h
	modificators/NewFuselageDialog.h
	modificators/NewConnectedElementDialog.h
	modificators/SymmetryComboBoxWidget.h
	modificators/ModificatorSectionsWidget.h
	modificators/ModificatorWingsWidget.h
	modificators/NewWingDialog.h
	ModificatorManager.h
	CPACSAbstractModel.h
	ModificatorContainerWidget.h
	CPACSTreeWidget.h
	CPACSFilterModel.h
	TIGLViewerNewFileDialog.h
)



set(tv_ui_comp
	src/TIGLViewerWindow.ui
	src/TIGLViewerSettingsDialog.ui
	src/TIGLViewerEtaXsiDialog.ui
	src/TIGLViewerErrorDialog.ui
	src/TIGLViewerFuseDialog.ui
	src/TIGLViewerShapeIntersectionDialog.ui
	src/TIGLViewerScreenshotDialog.ui
	src/TIGLViewerDrawVectorDialog.ui
	src/TIGLViewerVTKExportDialog.ui
	src/TIGLSliderDialog.ui
	src/TIGLGeometryChoserDialog.ui
   		src/modificators/ModificatorTransformationWidget.ui
		src/modificators/ModificatorTransformationWidget.ui
		src/modificators/ModificatorElementWidget.ui
		src/modificators/ModificatorWingWidget.ui
		src/modificators/ModificatorFuselageWidget.ui
		src/modificators/ModificatorSectionWidget.ui
		src/modificators/CTiglPointWidget.ui
		src/ModificatorContainerWidget.ui
		src/modificators/ModificatorFuselagesWidget.ui
		src/modificators/NewFuselageDialog.ui
		src/modificators/NewConnectedElementDialog.ui
		src/CPACSTreeWidget.ui
		src/TIGLViewerNewFileDialog.ui
		src/modificators/SymmetryComboBoxWidget.ui
		src/modificators/ModificatorSectionsWidget.ui
		src/modificators/ModificatorWingsWidget.ui
		src/modificators/NewWingDialog.ui

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
QTX_WRAP_CPP(tv_hdr_MOC ${tv_hdr_qobj_comp})
QTX_WRAP_UI(tv_ui_HEADERS ${tv_ui_comp})
# compile icons
QTX_ADD_RESOURCES( tv_rcrs src/TIGLViewer.qrc styles/qdarkstyle/style.qrc)

add_definitions(-DCSFDB -DQOCC_STATIC)

include_directories( ${OpenCASCADE_INCLUDE_DIR} ${TIXI_INCLUDE_DIRS})

# opengl
include_directories(${OPENGL_INCLUDE_DIR})

# boost
include_directories(${PROJECT_SOURCE_DIR}/thirdparty/boost_1_67_0)

include_directories( ${PROJECT_SOURCE_DIR}/src ${CMAKE_CURRENT_SOURCE_DIR}/src ${PROJECT_BINARY_DIR}/src)

# include path for ui files
include_directories( ${CMAKE_CURRENT_BINARY_DIR} )

source_group(MOCS REGULAR_EXPRESSION "moc_")
source_group(RESOURCES REGULAR_EXPRESSION "qrc_")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)

if(WIN32)
  # comment this line if you want to see an additional console window
  set(TYPE WIN32)

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
	tigl3_static
	${TIXI_LIBRARIES} 
	${CMake_QT_LIBRARIES} 
	${OPENGL_LIBRARIES} 
	${OpenCASCADE_LIBRARIES} 
	${EXTRA_LIBRARIES} 
)

if(GLOG_FOUND)
  include_directories(${GLOG_INCLUDE_DIR})
  target_link_libraries (${PROGNAME} ${GLOG_LIBRARIES})
endif(GLOG_FOUND)


#--------------------------------------------------------------------------------
# Now the data stuff below (templates files)
#--------------------------------------------------------------------------------

if (APPLE)
  install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/data/templates DESTINATION ${PROGNAME}.app/Contents/Resources COMPONENT viewer)
else()
  install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/data/templates DESTINATION share/tigl3 COMPONENT viewer)
endif()

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

ENDIF(WIN32)

SET(CMAKE_INSTALL_OPENMP_LIBRARIES "ON")
INCLUDE(InstallRequiredSystemLibraries)


# set directories to look for dependencies
set(DIRS ${TIXI_LIB_PATH} ${OpenCASCADE_DLL_DIRECTORY} ${OpenCASCADE_LINK_DIRECTORY} ${TIGL_ADD_LIB_PATH})

if (WIN32 OR APPLE)
  if(Qt5Widgets_FOUND)
    if(APPLE)
      set(QT_PLUGINS  "qcocoa")
    endif()
    if(WIN32)
      set(QT_PLUGINS  "qwindows")
    endif()
    set(QT_PLUGINS "${QT_PLUGINS};qjpeg")

    include(DeployQt5)
    INSTALL_QT5_EXECUTABLE(${APPS} "${QT_PLUGINS}" "" "${DIRS}" "" "" viewer)
  else()
    include(DeployQt4)
    INSTALL_QT4_EXECUTABLE(${APPS} "" "" "${DIRS}" "" "" viewer)
  endif()
endif()

# install shader files from opencascade
if (OpenCASCADE_SHADER_DIRECTORY)
   file(GLOB SHADER_FILES PATH "${OpenCASCADE_SHADER_DIRECTORY}/*")
   if (APPLE)
     install(FILES ${SHADER_FILES} DESTINATION ${PROGNAME}.app/Contents/Resources/ COMPONENT viewer)
   else()
     install(FILES ${SHADER_FILES} DESTINATION share/tigl3/shaders COMPONENT viewer)
   endif(APPLE)
endif()

else (QT4_FOUND OR Qt5Widgets_FOUND)
message(STATUS "No qt was found. TiGL Viewer will not be build!")
endif(QT4_FOUND OR Qt5Widgets_FOUND)

endif(TIGL_VIEWER)


