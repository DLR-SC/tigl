# - Locate FreeType library
# This module defines
#  FREETYPE_LIBRARIES, the library to link against
#  FREETYPE_FOUND, if false, do not try to link to FREETYPE
#
# $FREETYPE_DIR is an environment variable that would
# correspond to the ./configure --prefix=$FREETYPE_DIR
# used in building FREETYPE.

#=============================================================================
# Copyright 2007-2009 Kitware, Inc.
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

# Created by Eric Wing.
# Modifications by Alexander Neundorf.
# This file has been renamed to "FindFreetypeLib.cmake" since it is only
# used to locate the library and no include files

find_library(FREETYPE_LIBRARY
  NAMES freetype libfreetype freetype219 freetype253
  HINTS
    ENV FREETYPE_DIR
  PATH_SUFFIXES lib
  PATHS
    /usr/X11R6
    /usr/local/X11R6
    /usr/local/X11
    /usr/freeware
    ENV GTKMM_BASEPATH
    [HKEY_CURRENT_USER\\SOFTWARE\\gtkmm\\2.4;Path]
    [HKEY_LOCAL_MACHINE\\SOFTWARE\\gtkmm\\2.4;Path]
)

# set the user variables
set(FREETYPE_LIBRARIES "${FREETYPE_LIBRARY}")


# handle the QUIETLY and REQUIRED arguments and set FREETYPE_FOUND to TRUE if
# all listed variables are TRUE
include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(Freetype
                                  REQUIRED_VARS FREETYPE_LIBRARY )

mark_as_advanced(FREETYPE_LIBRARY)
