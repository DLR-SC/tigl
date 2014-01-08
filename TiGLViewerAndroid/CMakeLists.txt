include(android-tools)

find_ndk_mod(osg)
find_ndk_mod(oce)
find_ndk_mod(tixi)
find_ndk_mod(curl)
find_ndk_mod(libxml2)
find_ndk_mod(libxslt)


if (NOT "${PROJECT_SOURCE_DIR}" STREQUAL "${PROJECT_BINARY_DIR}")
  message(FATAL_ERROR "Out of source build not supported for android build")
endif()


IF(CMAKE_BUILD_TYPE MATCHES Debug)
  SET(MODE "debug")
ELSE()
  SET(MODE "release")
ENDIF()


configure_file (
  "${PROJECT_SOURCE_DIR}/src/tigl_version.h.in"
  "${PROJECT_SOURCE_DIR}/src/tigl_version.h"
)

configure_file (
  "${CMAKE_CURRENT_SOURCE_DIR}/local.properties.in"
  "${CMAKE_CURRENT_SOURCE_DIR}/local.properties"
)

configure_file (
  "${CMAKE_CURRENT_SOURCE_DIR}/jni/Application.mk.in"
  "${CMAKE_CURRENT_SOURCE_DIR}/jni/Application.mk"
)

ADD_CUSTOM_COMMAND(
  OUTPUT    tiglviewer-native
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  COMMAND   "${ANDROID_NDK}/ndk-build"
  ARGS      -j 8 
)

ADD_CUSTOM_COMMAND(
  OUTPUT    ${CMAKE_CURRENT_BINARY_DIR}/bin/TiGLViewer-${MODE}.apk
  DEPENDS   tiglviewer-native
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  COMMAND   "${ANT_EXECUTABLE}"
  ARGS      "${MODE}" 
)

ADD_CUSTOM_TARGET(
  TiGLViewerApp ALL 
  DEPENDS bin/TiGLViewer-${MODE}.apk
)

message("The TiGLViewer app can now be build using \"make\"")

