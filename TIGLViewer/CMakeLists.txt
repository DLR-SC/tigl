# TIGLViewer cmake project
# author: Martin Siggel

# set up QT

OPTION(TIGL_VIEWER "Builds the TiGL Viewer program (requires OpenGL and Qt4)" ON)


if (APPLE)
   set(PROGNAME "CPACS-Creator")
else(APPLE)
   set(PROGNAME "cpacscreator")
endif(APPLE)


if (TIGL_VIEWER)

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

set(tv_src
  src/ISession_Point.cpp
  src/ISession_Direction.cpp
  src/ISession_Text.cpp
  src/main.cpp
  src/TIGLViewerContext.cpp
  src/TIGLViewerDocument.cpp
  src/TIGLViewerInputoutput.cpp
  src/TIGLViewerInternal.h.cpp
  src/TIGLViewerWidget.cpp
  src/TIGLViewerWindow.cpp
  src/TIGLViewerSettingsDialog.cpp
  src/TIGLScriptProxy.cpp
  src/TIXIScriptProxy.cpp
  src/TIGLScriptEngine.cpp
  src/TiglViewerConsole.cpp
  src/TIGLViewerSettings.cpp
  src/TIGLViewerControlFile.cpp
  src/TIGLViewerEtaXsiDialog.cpp
  src/CHotsoseMeshReader.cpp
  src/TIGLAISTriangulation.cpp
  src/TIGLViewerErrorDialog.cpp
  src/TIGLViewerLogHistory.cpp
  src/TIGLViewerLogRedirection.cpp
  src/TIGLViewerLoggerHTMLDecorator.cpp
  src/TIGLViewerFuseDialog.cpp
  src/TIGLViewerShapeIntersectionDialog.cpp
  src/TIGLViewerScreenshotDialog.cpp
  src/TIGLViewerDrawVectorDialog.cpp
  src/TIGLViewerScopedCommand.cpp
  src/TIGLQAspectWindow.cpp
  src/TIGLViewerVTKExportDialog.cpp
  src/TIGLSliderDialog.cpp
  src/TIGLViewerApp.cpp
  src/TIGLGeometryChoserDialog.cpp
  src/TIGLViewerUndoCommands.cpp
  src/ProfilesDBManager.cpp
  # files for CPACSCreator
  src/modificators/ModificatorWidget.cpp
  src/modificators/ModificatorElementWidget.cpp
  src/modificators/ModificatorSectionWidget.cpp
  src/modificators/ModificatorFuselageWidget.cpp
  src/modificators/ModificatorTransformationWidget.cpp
  src/modificators/ModificatorWingWidget.cpp
  src/modificators/CTiglPointWidget.cpp
  src/modificators/ModificatorFuselagesWidget.cpp
  src/modificators/NewFuselageDialog.cpp
  src/modificators/NewConnectedElementDialog.cpp
  src/modificators/SymmetryComboBoxWidget.cpp
  src/modificators/ModificatorSectionsWidget.cpp
  src/modificators/ModificatorWingsWidget.cpp
  src/modificators/NewWingDialog.cpp
  src/modificators/DeleteDialog.cpp
  src/modificators/ModificatorPositioningWidget.cpp
  src/StandardizeDialog.cpp
  src/ModificatorManager.cpp
  src/CPACSAbstractModel.cpp
  src/CPACSTreeItem.cpp
  src/CPACSTree.cpp
  src/XPathParser.cpp
  src/TIGLViewerException.cpp
  src/ModificatorContainerWidget.cpp
  src/CPACSTreeWidget.cpp
  src/CPACSFilterModel.cpp
  src/TIGLViewerNewFileDialog.cpp
  src/TIGLViewerCommonFunctions.cpp
)
	
# header files
set(tv_hdr
  src/CommandLineParameters.h
  src/ISession_Point.h
  src/ISession_Direction.h
  src/ISession_Text.h
  src/TIGLViewer.h
  src/TIGLViewerSettings.h
  src/TIGLViewerControlFile.h
  src/TIGLAISTriangulation.h
  src/CHotsoseMeshReader.h
  src/TIGLViewerLogHistory.h
  src/TIGLViewerLoggerHTMLDecorator.h
  src/TIGLViewerScopedCommand.h
  src/TIGLQAspectWindow.h
  src/TIGLViewerUndoCommands.h
  src/ProfilesDBManager.h
  src/CPACSTreeItem.h
  src/CPACSTree.h
  src/XPathParser.h
  src/TIGLViewerException.h
  src/TIGLViewerCommonFunctions.h
  src/TIGLDebugStream.h
  src/TIGLViewerContext.h
  src/TIGLViewerDocument.h
  src/TIGLViewerInputoutput.h
  src/TIGLViewerInputoutput.h
  src/TIGLViewerWidget.h
  src/TIGLViewerWindow.h
  src/TIGLViewerSettingsDialog.h
  src/TIGLScriptProxy.h
  src/TIXIScriptProxy.h
  src/TIGLScriptEngine.h
  src/TiglViewerConsole.h
  src/TIGLViewerEtaXsiDialog.h
  src/TIGLViewerErrorDialog.h
  src/TIGLViewerDrawVectorDialog.h
  src/TIGLViewerLogRedirection.h
  src/TIGLViewerFuseDialog.h
  src/TIGLViewerShapeIntersectionDialog.h
  src/TIGLViewerScreenshotDialog.h
  src/TIGLViewerVTKExportDialog.h
  src/TIGLSliderDialog.h
  src/TIGLViewerApp.h
  src/TIGLGeometryChoserDialog.h
  src/modificators/ModificatorWidget.h
  src/modificators/CTiglPointWidget.h
  src/modificators/ModificatorElementWidget.h
  src/modificators/ModificatorSectionWidget.h
  src/modificators/ModificatorFuselageWidget.h
  src/modificators/ModificatorTransformationWidget.h
  src/modificators/ModificatorWingWidget.h
  src/modificators/ModificatorFuselagesWidget.h
  src/modificators/NewFuselageDialog.h
  src/modificators/NewConnectedElementDialog.h
  src/modificators/SymmetryComboBoxWidget.h
  src/modificators/ModificatorSectionsWidget.h
  src/modificators/ModificatorWingsWidget.h
  src/modificators/NewWingDialog.h
  src/modificators/DeleteDialog.h
  src/modificators/ModificatorPositioningWidget.h
  src/ModificatorManager.h
  src/CPACSAbstractModel.h
  src/ModificatorContainerWidget.h
  src/CPACSTreeWidget.h
  src/CPACSFilterModel.h
  src/TIGLViewerNewFileDialog.h
  src/StandardizeDialog.h
)

set(tv_ui
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
  src/StandardizeDialog.ui
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
  src/modificators/DeleteDialog.ui
  src/modificators/ModificatorPositioningWidget.ui
)

# compile icons
QTX_ADD_RESOURCES( tv_rcrs src/TIGLViewer.qrc styles/qdarkstyle/style.qrc)

source_group(MOCS REGULAR_EXPRESSION "moc_")
source_group(RESOURCES REGULAR_EXPRESSION "qrc_")

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
  ${tv_src} 
  ${tv_hdr}
  ${tv_ui}
  ${tv_rcrs}
  ${tv_icon_rc}
)

set_target_properties(${PROGNAME} PROPERTIES AUTOMOC TRUE)
set_target_properties(${PROGNAME} PROPERTIES AUTOUIC TRUE)

# opengl
target_include_directories(${PROGNAME} PRIVATE ${OPENGL_INCLUDE_DIR})
# include path for ui files
target_include_directories(${PROGNAME} PRIVATE ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/src)

target_link_libraries(${PROGNAME} PRIVATE
  tigl3_static tigl3_cpp
  ${CMake_QT_LIBRARIES} 
  ${OPENGL_LIBRARIES} 
  ${OpenCASCADE_LIBRARIES} 
  ${EXTRA_LIBRARIES} 
)


#--------------------------------------------------------------------------------
# Now the data stuff below (templates files)
#--------------------------------------------------------------------------------

if (APPLE)
  install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/data/templates DESTINATION ${PROGNAME}.app/Contents/Resources COMPONENT viewer)
  install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/data/profiles DESTINATION ${PROGNAME}.app/Contents/Resources COMPONENT viewer)
else()
  install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/data/templates DESTINATION share/tigl3 COMPONENT viewer)
  install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/data/profiles DESTINATION share/tigl3 COMPONENT viewer)
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
set(DEPENDS_PATH "lib")
IF(APPLE)
  set(APPS "${PROGNAME}.app")
  set(DEPENDS_PATH "${PROGNAME}.app/Contents/MacOS/")
  
  # TKOpenGL is loaded as a plugin, hence we must manually install it
  INSTALL(FILES ${TKOpenGl_OCCLIB} DESTINATION ${PROGNAME}.app/Contents/MacOS/ COMPONENT viewer)
  # copy icon
  INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/gfx/${PROGNAME}.icns DESTINATION ${PROGNAME}.app/Contents/Resources/ COMPONENT viewer)
ENDIF(APPLE)
IF(WIN32)
  set(APPS "bin/${PROGNAME}.exe")
  set(DEPENDS_PATH "bin")

  # install runtime libs
  FILE(TO_CMAKE_PATH "$ENV{SYSTEMROOT}" SYSTEMROOT)

ENDIF(WIN32)

SET(CMAKE_INSTALL_OPENMP_LIBRARIES "ON")
INCLUDE(InstallRequiredSystemLibraries)


function (get_dll_path DLL_PATH target)
   if (TARGET ${target})
    get_target_property(TARGET_DLL ${target} LOCATION)
    get_filename_component(TMP_DLL_PATH ${TARGET_DLL} PATH)
    set (${DLL_PATH} ${TMP_DLL_PATH} PARENT_SCOPE)
  endif()
endfunction ()

get_dll_path(OCC_DLL_PATH TKernel)
get_dll_path(TIXI_DLL_PATH tixi3)
get_dll_path(GLOG_DLL_PATH glog::glog)

# set directories to look for dependencies
set(DIRS ${OCC_DLL_PATH} ${TIXI_DLL_PATH} ${GLOG_DLL_PATH})

if (WIN32 OR APPLE)
  install(FILES $<TARGET_FILE:tixi3> DESTINATION ${DEPENDS_PATH} COMPONENT viewer)

  if (TARGET glog::glog)
     install(FILES $<TARGET_FILE:glog::glog> DESTINATION ${DEPENDS_PATH} COMPONENT viewer) 
  endif()
  
  if(Qt5Widgets_FOUND)
    if(APPLE)
      set(QT_PLUGINS  "qcocoa")
    endif()
    if(WIN32)
      set(QT_PLUGINS  "qwindows")
    endif()
    set(QT_PLUGINS "${QT_PLUGINS};qjpeg")

    install(FILES
        $<TARGET_FILE:Qt5::Core>
        $<TARGET_FILE:Qt5::Widgets>
        $<TARGET_FILE:Qt5::Gui>
        $<TARGET_FILE:Qt5::Script>
    DESTINATION ${DEPENDS_PATH} COMPONENT viewer)

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


