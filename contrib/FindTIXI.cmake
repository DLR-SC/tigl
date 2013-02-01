# - Find tixi
# Find the native TIXI headers and libraries.
#
#  TIXI_INCLUDE_DIRS   - where to find libtixi/tixi.h, etc.
#  TIXI_LIBRARIES      - List of libraries when using tixi.
#  TIXI_FOUND          - True if tixi found.
#  TIXI_VERSION_STRING - the version of tixi found (since CMake 2.8.8)

#=============================================================================
# Copyright 2006-2009 Kitware, Inc.
# Copyright 2012 Rolf Eike Beer <eike@sf-mail.de>
#
# Distributed under the OSI-approved BSD License (the "License");
# see accompanying file Copyright.txt for details.
#
# This software is distributed WITHOUT ANY WARRANTY; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the License for more information.
#=============================================================================
# (To distribute this file outside of CMake, substitute the full
#  License text for the above reference.)

# Look for the header file.
FIND_PATH(TIXI_INCLUDE_DIR 
	NAMES tixi.h
	HINTS
	${TIXI_PATH}/include
	PATHS
	"[HKEY_LOCAL_MACHINE\\SOFTWARE\\www.dlr.de/sc\\TIXI]/include" 
)
	
# MARK_AS_ADVANCED(TIXI_INCLUDE_DIR)

# Look for the library.
FIND_LIBRARY(TIXI_LIBRARY NAMES 
	TIXI
	HINTS
	${TIXI_PATH}/lib64
	${TIXI_PATH}/lib
	PATHS
	"[HKEY_LOCAL_MACHINE\\SOFTWARE\\www.dlr.de/sc\\TIXI]/lib"
)
# MARK_AS_ADVANCED(TIXI_LIBRARY)

if(WIN32)
FIND_PATH(TIXI_BINARY_DIR 
	NAMES TIXI.dll
	HINTS
	${TIXI_PATH}/bin
	PATHS
	"[HKEY_LOCAL_MACHINE\\SOFTWARE\\www.dlr.de/sc\\TIXI]/bin"
)
else(WIN32)
GET_FILENAME_COMPONENT(TIXI_BINARY_DIR ${TIXI_LIBRARY} PATH)
endif(WIN32)

# handle the QUIETLY and REQUIRED arguments and set TIXI_FOUND to TRUE if 
# all listed variables are TRUE
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(TIXI
                                  REQUIRED_VARS TIXI_LIBRARY TIXI_INCLUDE_DIR
                                  VERSION_VAR TIXI_VERSION_STRING)

IF(TIXI_FOUND)
  SET(TIXI_LIBRARIES ${TIXI_LIBRARY})
  SET(TIXI_INCLUDE_DIRS ${TIXI_INCLUDE_DIR})
ELSE(TIXI_FOUND)
   message(WARNING "TIXI not found. Try set TIXI_PATH variable")
ENDIF(TIXI_FOUND)