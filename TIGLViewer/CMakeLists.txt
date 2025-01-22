# TIGLViewer cmake project
# author: Martin Siggel

# set up QT

OPTION(TIGL_VIEWER "Builds the TiGL Viewer program (requires OpenGL and Qt4)" ON)


if (APPLE)
   set(PROGNAME "TiGL-Viewer3")
else(APPLE)
   set(PROGNAME "tiglviewer-3")
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
endif(OCE_FOUND)

# we need opengl in order to build tiglviewer
cmake_policy(SET CMP0072 NEW)
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

# get source files, headers and UI files
file(GLOB_RECURSE tv_src "src/*.cpp")
file(GLOB_RECURSE tv_hdr "src/*.h")
file(GLOB_RECURSE tv_ui "src/*.ui")

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

# include path for ui files
target_include_directories(${PROGNAME} PRIVATE ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/src)

# workaround: on macos, the file glext.h should be loaded from occt/include instead from the system dir
# This is a "Bug" in the occt header files which use <> instead of "" for include
# hence, we must explicitly inlude the occt dir before all other includes!
target_include_directories(${PROGNAME} BEFORE PRIVATE  ${OpenCASCADE_INCLUDE_DIR}) 

target_link_libraries(${PROGNAME} PRIVATE
  tigl3_static tigl3_cpp
  ${CMake_QT_LIBRARIES} 
  ${OpenCASCADE_LIBRARIES} 
  OpenGL::GL
  ${EXTRA_LIBRARIES} 
)

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


