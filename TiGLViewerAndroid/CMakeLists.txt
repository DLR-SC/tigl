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
  SET(TASK "assembleDebug")
ELSE()
  SET(MODE "release")
  SET(TASK "assembleRelease")
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

if(NOT WIN32)
    ADD_CUSTOM_COMMAND(
        OUTPUT    TiGLViewer.apk
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMAND   "${CMAKE_CURRENT_SOURCE_DIR}/gradlew"
        ARGS      "${TASK}" 
    )
    else()
    ADD_CUSTOM_COMMAND(
        OUTPUT    TiGLViewer.apk
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMAND   "${CMAKE_CURRENT_SOURCE_DIR}/gradle.bat"
        ARGS      "${TASK}" 
    )
endif()

ADD_CUSTOM_TARGET(
  TiGLViewerApp ALL 
  DEPENDS TiGLViewer.apk
)

message("The TiGLViewer app can now be build using \"make\"")

