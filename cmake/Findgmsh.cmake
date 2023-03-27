# -*- mode: cmake -*-
#
#  This file is part of the Feel library
#
#  Author(s): Christophe Prud'homme <christophe.prudhomme@ujf-grenoble.fr>
#       Date: 2010-07-28
#
#  Copyright (C) 2010 Université de Grenoble 1 (Joseph Fourier)
#
#  This library is free software; you can redistribute it and/or
#  modify it under the terms of the GNU Lesser General Public
#  License as published by the Free Software Foundation; either
#  version 3.0 of the License, or (at your option) any later version.
#
#  This library is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#  Lesser General Public License for more details.
#
#  You should have received a copy of the GNU Lesser General Public
#  License along with this library; if not, write to the Free Software
#  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
#
include (FindPackageHandleStandardArgs)
INCLUDE(CheckIncludeFileCXX)



FIND_PATH(GMSH_INCLUDE_DIR
  gmsh.h gmshc.h
  PATHS ${CMAKE_SYSTEM_PREFIX_PATH} ${GMSH_DIR}
  PATH_SUFFIXES  include/
  DOC "Directory where GMSH header files are stored"
)
message("gmsh dir = ${GMSH_DIR}")
include_directories(${GMSH_INCLUDE_DIR})

FIND_LIBRARY(GMSH_LIBRARY 
  gmsh libgmsh
  PATHS ${CMAKE_SYSTEM_PREFIX_PATH} ${GMSH_DIR}
  PATH_SUFFIXES  lib  
)
if( NOT GMSH_LIBRARY )
 FIND_PATH(GMSH_LIBRARY_PATH
  libGmsh.so
  PATHS ${CMAKE_SYSTEM_PREFIX_PATH} ${GMSH_DIR}/
  PATH_SUFFIXES install/lib )
  set(GMSH_LIBRARY "${GMSH_LIBRARY_PATH}/libGmsh.so" )
endif()

FIND_LIBRARY(GL2PS_LIBRARY NAMES gl2ps
   PATH
   ${CMAKE_SYSTEM_PREFIX_PATH}
   ${GMSH_DIR}/install/lib
   PATH_SUFFIXES
   install/lib  )
FIND_LIBRARY(GL_LIBRARY NAMES GL
   PATH
   ${CMAKE_SYSTEM_PREFIX_PATH}
   ${GMSH_DIR}/lib
   PATH_SUFFIXES
   lib  )

find_program( GMSH_EXECUTABLE gmsh DOC "GMSH mesh generator" )

# handle the QUIETLY and REQUIRED arguments and set Madlib_FOUND to TRUE if
# all listed variables are TRUE
FIND_PACKAGE_HANDLE_STANDARD_ARGS (GMSH DEFAULT_MSG
  GMSH_INCLUDE_DIR GMSH_LIBRARY GMSH_EXECUTABLE 
  )

if ( GMSH_FOUND )
  MESSAGE( STATUS "GMSH found: header(${GMSH_INCLUDE_DIR}) lib(${GMSH_LIBRARY}) executable(${GMSH_EXECUTABLE})" )
  MESSAGE( STATUS "GL2PS found: lib(${GL2PS_LIBRARY})" )
  MESSAGE( STATUS "GL found: lib(${GL_LIBRARY})" )
endif()

mark_as_advanced( GMSH_INCLUDE_DIR )
mark_as_advanced( GMSH_LIBRARY )
mark_as_advanced( GL2PS_LIBRARY )
mark_as_advanced( GL_LIBRARY )
mark_as_advanced( GMSH_EXECUTABLE )