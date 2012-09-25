# - Find xml
# Find the native CUNIT headers and libraries.
#
#  CUNIT_INCLUDE_DIRS   - where to find libxml/xml.h, etc.
#  CUNIT_LIBRARIES      - List of libraries when using xml.
#  CUNIT_FOUND          - True if xml found.
#  CUNIT_VERSION_STRING - the version of xml found (since CMake 2.8.8)

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
FIND_PATH(CUNIT_INCLUDE_DIR NAMES CUnit/CUnit.h)
MARK_AS_ADVANCED(CUNIT_INCLUDE_DIR)

# Look for the library.
FIND_LIBRARY(CUNIT_LIBRARY NAMES 
	cunit
	libcunit_vc
)
MARK_AS_ADVANCED(CUNIT_LIBRARY)

# handle the QUIETLY and REQUIRED arguments and set CUNIT_FOUND to TRUE if 
# all listed variables are TRUE
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(CUNIT
                                  REQUIRED_VARS CUNIT_LIBRARY CUNIT_INCLUDE_DIR
                                  VERSION_VAR CUNIT_VERSION_STRING)

IF(CUNIT_FOUND)
  SET(CUNIT_LIBRARIES ${CUNIT_LIBRARY})
  SET(CUNIT_INCLUDE_DIRS ${CUNIT_INCLUDE_DIR})
ENDIF(CUNIT_FOUND)
