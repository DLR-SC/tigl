###  FindGLOG.cmake; coding: utf-8 ---
#
# GLOG_FOUND - system has GLOG
# GLOG_INCLUDE_DIR - headers location
# GLOG_LIBRARIES - libraries

# try to find glog headers, if not found then install glog from contrib into
# build directory and set GLOG_INCLUDE_DIR and GLOG_LIBRARIES
FIND_PATH(GLOG_INCLUDE_DIR glog/logging.h
  ${CMAKE_BINARY_DIR}/thirdparty/glog/include
  /opt/local/include
  /usr/local/include
  /usr/include
  )
  
message(STATUS "Glog first pass: ${GLOG_INCLUDE_DIR}")
if (NOT GLOG_INCLUDE_DIR )
  message(STATUS "Building glog in ${CMAKE_BINARY_DIR}/thirdparty/glog...")
  execute_process(COMMAND mkdir -p ${CMAKE_BINARY_DIR}/thirdparty/glog)
  execute_process(
    COMMAND ${PROJECT_SOURCE_DIR}/thirdparty/glog-0.3.3/configure --prefix=${CMAKE_BINARY_DIR}/thirdparty/glog
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/thirdparty/glog-0.3.3
    OUTPUT_QUIET
    OUTPUT_FILE "glog"
    )
  message(STATUS "Installing glog in ${CMAKE_BINARY_DIR}/thirdparty/glog...")
  execute_process(
    COMMAND make install
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/thirdparty/glog-0.3.3
    OUTPUT_QUIET
    )
endif()

FIND_LIBRARY(GLOG_LIBRARY
  NAMES glog
  PATHS
  ${CMAKE_BINARY_DIR}/thirdparty/glog/lib/
  /opt/local/lib
  /usr/local/lib
  /usr/lib
  )
FIND_PATH(GLOG_INCLUDE_DIR glog/logging.h
  ${CMAKE_BINARY_DIR}/thirdparty/glog/include
  /opt/local/include
  /usr/local/include
  /usr/include
  )
  
set(GLOG_LIBRARIES ${GLOG_LIBRARY})
set(GLOG_INCLUDE_DIR ${CMAKE_BINARY_DIR}/thirdparty/glog/include)
message(STATUS "GLog includes: ${GLOG_INCLUDE_DIR} Libraries: ${GLOG_LIBRARIES}" )

# handle the QUIETLY and REQUIRED arguments and set OpenTURNS_FOUND to TRUE if
# all listed variables are TRUE
include (FindPackageHandleStandardArgs)
find_package_handle_standard_args (GLOG DEFAULT_MSG GLOG_INCLUDE_DIR GLOG_LIBRARIES )

mark_as_advanced (GLOG_INCLUDE_DIR GLOG_LIBRARIES)