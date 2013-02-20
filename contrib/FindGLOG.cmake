###  FindGLOG.cmake; coding: utf-8 ---
#
# GLOG_FOUND - system has GLOG
# GLOG_INCLUDE_DIR - headers location
# GLOG_LIBRARIES - libraries

# try to find glog headers, if not found then install glog from contrib into
# build directory and set GLOG_INCLUDE_DIR and GLOG_LIBRARIES
FIND_PATH(GLOG_INCLUDE_DIR 
  NAMES glog/logging.h
  PATHS
  /opt/local/include
)

FIND_LIBRARY(GLOG_LIBRARY
  NAMES glog 
  # and for windows
  libglog
  PATHS
  /opt/local/lib
  /usr/local/lib
  /usr/lib
  )

# handle the QUIETLY and REQUIRED arguments and set GLOG_FOUND to TRUE if
# all listed variables are TRUE
include (FindPackageHandleStandardArgs)
find_package_handle_standard_args (GLOG REQUIRED_VARS GLOG_INCLUDE_DIR GLOG_LIBRARY )

IF(GLOG_FOUND)
  SET(GLOG_LIBRARIES ${GLOG_LIBRARY})
ENDIF(GLOG_FOUND)
mark_as_advanced (GLOG_INCLUDE_DIR GLOG_LIBRARY)
