##############################################################################
# @file  FindMATLAB.cmake
# @brief Find MATLAB installation.
#
 # @par Input variables:
 # <table border="0">
 #   <tr>
 #     @tp @b MATLAB_DIR @endtp
 #     <td>The installation directory of MATLAB.
 #         Can also be set as environment variable.</td>
 #   </tr>
 #   <tr>
 #     @tp @b MATLABDIR @endtp
 #     <td>Alternative environment variable for @p MATLAB_DIR.</td>
 #   </tr>
 #   <tr>
 #     @tp @b MATLAB_FIND_COMPONENTS @endtp
 #     <td>The @c COMPONENTS argument(s) of the find_package() command can
 #         be used to only look for specific MATLAB executables and libraries.
 #         Valid component values are "matlab", "mcc", "mexext", "mex",
 #         "libmex", "mx" or "libmx", and "eng" or "libeng".</td>
 #   </tr>
 #   <tr>
 #     @tp @b MATLAB_PATH_SUFFIXES @endtp
 #     <td>Path suffixes which are used to find the proper MATLAB libraries.
 #         By default, this find module tries to determine the path suffix
 #         from the CMake variables which describe the system. For example,
 #         on 64-bit Unix-based systems, the libraries are searched in
 #         @p MATLAB_DIR/bin/glnxa64. Set this variable before the
 #         find_package() command if this find module fails to
 #         determine the correct location of the MATLAB libraries within
 #         the root directory.</td>
 #   </tr>
 # </table>
 #
 # @par Output variables:
 # <table border="0">
 #   <tr>
 #     @tp @b MATLAB_FOUND @endtp
 #     <td>Whether the package was found and the following CMake
 #         variables are valid.</td>
 #   </tr>
 #   <tr>
 #     @tp @b MATLAB_EXECUTABLE @endtp
 #     <td>The absolute path of the found matlab executable.</td>
 #   </tr>
 #   <tr>
 #     @tp @b MATLAB_MCC_EXECUTABLE @endtp
 #     <td>The absolute path of the found MATLAB Compiler (mcc) executable.</td>
 #   </tr>
 #   <tr>
 #     @tp @b MATLAB_MEX_EXECUTABLE @endtp
 #     <td>The absolute path of the found MEX script (mex) executable.</td>
 #   </tr>
 #   <tr>
 #     @tp @b MATLAB_MEXEXT_EXECUTABLE @endtp
 #     <td>The absolute path of the found mexext script executable.</td>
 #   </tr>
 #   <tr>
 #     @tp @b MATLAB_INCLUDE_DIR @endtp
 #     <td>Package include directories.</td>
 #   </tr>
 #   <tr>
 #     @tp @b MATLAB_INCLUDES @endtp
 #     <td>Include directories including prerequisite libraries.</td>
 #   </tr>
 #   <tr>
 #     @tp @b MATLAB_mex_LIBRARY @endtp
 #     <td>The MEX library of MATLAB.</td>
 #   </tr>
 #   <tr>
 #     @tp @b MATLAB_mx_LIBRARY @endtp
 #     <td>The @c mx library of MATLAB.</td>
 #   </tr>
 #   <tr>
 #     @tp @b MATLAB_eng_LIBRARY @endtp
 #     <td>The MATLAB engine library.</td>
 #   </tr>
 #   <tr>
 #     @tp @b MATLAB_LIBRARY @endtp
 #     <td>All MATLAB libraries.</td>
 #   </tr>
 #   <tr>
 #     @tp @b MATLAB_LIBRARIES @endtp
 #     <td>Package libraries and prerequisite libraries.</td>
 #   </tr>
 # </table>
 #
 # Copyright (c) 2011, 2012 University of Pennsylvania. All rights reserved.<br />
 # See http://www.rad.upenn.edu/sbia/software/license.html or COPYING file.
 #
 # Contact: SBIA Group <sbia-software at uphs.upenn.edu>
 #
 # @ingroup CMakeFindModules
 ##############################################################################
 
 # ----------------------------------------------------------------------------
 # initialize search
 if (NOT MATLAB_DIR)
   if (NOT $ENV{MATLABDIR} STREQUAL "")
     set (MATLAB_DIR "$ENV{MATLABDIR}"  CACHE PATH "Installation prefix for MATLAB." FORCE)
   else ()
     set (MATLAB_DIR "$ENV{MATLAB_DIR}" CACHE PATH "Installation prefix for MATLAB." FORCE)
   endif ()
 endif ()
 
 if(NOT MATLAB_LIB_DIR)
	set(MATLAB_LIB_DIR "$ENV{MATLAB_LIB_DIR}" CACHE PATH "Library path for MATLAB mex files." FORCE)
 endif()
 
# Determine Bitness of build 
set (BITS 32)
if(${CMAKE_SIZEOF_VOID_P} EQUAL 4)
	set(BITS 32)
elseif(${CMAKE_SIZEOF_VOID_P} EQUAL 8)
	set(BITS 64)
else()
	message(SEND_ERROR "Platform could not be detected")
endif()

IF(UNIX)
    # MAC
    IF("${CMAKE_SYSTEM_NAME}" MATCHES "Darwin")
        IF( ${CMAKE_OSX_ARCHITECTURES} MATCHES "i386" )
            SET( MATLAB_ARCH maci )
            SET( MATLAB_MEX_SUFFIX .mexmaci CACHE STRING "Mex shared library file suffix" )
            SET( MATLAB_CXX_FLAGS
                    "${CMAKE_CXX_FLAGS} -flat_namespace -undefined suppress" CACHE INTERNAL 
                    "extra CFLAGS to suppress linker errors in Mac OS X" )
        ELSE()
            SET( MATLAB_ARCH maci64 )
            SET( MATLAB_MEX_SUFFIX .mexmaci64)
        ENDIF()
    ENDIF()
    # LINUX
    IF("${CMAKE_SYSTEM_NAME}" MATCHES "Linux")
        SET( MATLAB_ARCH glnx86 )
	IF(BITS EQUAL 64)
		SET( MATLAB_MEX_SUFFIX .mexa64 )
	ELSE()
	        SET( MATLAB_MEX_SUFFIX .mexglx )
	ENDIF()
    ENDIF()
ELSEIF(WIN32)
	if(BITS EQUAL 64)
		SET( MATLAB_MEX_SUFFIX .mexw64 )
		SET( MATLAB_ARCH win64 )
	else()
		SET( MATLAB_MEX_SUFFIX .mexw32 )
		SET( MATLAB_ARCH win32 )
	endif()
ENDIF()

 if (NOT MATLAB_PATH_SUFFIXES)
   if (WIN32)
     if (CMAKE_GENERATOR MATCHES "Visual Studio 6")
       set (MATLAB_PATH_SUFFIXES "extern/lib/win32/microsoft/msvc60")
     elseif (CMAKE_GENERATOR MATCHES "Visual Studio 7")
       # assume people are generally using 7.1,
       # if using 7.0 need to link to: extern/lib/win32/microsoft/msvc70
       set (MATLAB_PATH_SUFFIXES "extern/lib/win32/microsoft/msvc71")
     elseif (CMAKE_GENERATOR MATCHES "Visual Studio 8")
       set (MATLAB_PATH_SUFFIXES "extern/lib/win32/microsoft/msvc80")
     elseif (CMAKE_GENERATOR MATCHES "Visual Studio 9")
       set (MATLAB_PATH_SUFFIXES "extern/lib/win32/microsoft/msvc90")
	 elseif (CMAKE_GENERATOR MATCHES "Visual Studio 10")
       set (MATLAB_PATH_SUFFIXES "extern/lib/win32/microsoft/msvc100")
     elseif (CMAKE_GENERATOR MATCHES "Borland")
       # assume people are generally using 5.4
       # if using 5.0 need to link to: ../extern/lib/win32/microsoft/bcc50
       # if using 5.1 need to link to: ../extern/lib/win32/microsoft/bcc51
       set (MATLAB_PATH_SUFFIXES "extern/lib/win32/microsoft/bcc54")
     endif ()
   else ()
     if (CMAKE_SIZE_OF_VOID_P EQUAL 4)
       set (MATLAB_PATH_SUFFIXES "bin/glnx86")
     else ()
       set (MATLAB_PATH_SUFFIXES "bin/glnxa64")
     endif ()
   endif ()
 endif ()
 
set (_MATLAB_EXECUTABLE_NAMES matlab mcc mex mexext)
set (_MATLAB_LIBRARY_NAMES    mex mx mat)

 # ----------------------------------------------------------------------------
 # find paths/files
 if (_MATLAB_LIBRARY_NAMES)
   if (MATLAB_DIR)
 
     find_path (
       MATLAB_INCLUDE_DIR
         NAMES mex.h
         HINTS "${MATLAB_DIR}/extern/include"
         DOC   "Include directory for MATLAB libraries."
         NO_DEFAULT_PATH
     )
 
     foreach (_MATLAB_LIB IN LISTS _MATLAB_LIBRARY_NAMES)
       find_library (
         MATLAB_${_MATLAB_LIB}_LIBRARY
           NAMES         "${_MATLAB_LIB}" "lib${_MATLAB_LIB}"
           PATHS         "${MATLAB_LIB_DIR}" "${MATLAB_DIR}"
           PATH_SUFFIXES  ${MATLAB_PATH_SUFFIXES} "bin/${MATLAB_ARCH}/" "extern/lib/${MATLAB_ARCH}/microsoft"	 "bin/${MATLAB_ARCH}"
           DOC           "MATLAB ${_MATLAB_LIB} link library."
           NO_DEFAULT_PATH
           NO_CMAKE_SYSTEM_PATH
       )
     endforeach ()
 
   else ()
 
     find_path (
       MATLAB_INCLUDE_DIR
         NAMES mex.h
         HINTS ENV C_INCLUDE_PATH ENV CXX_INCLUDE_PATH
         DOC   "Include directory for MATLAB libraries."
     )
 
     foreach (_MATLAB_LIB IN LISTS _MATLAB_LIBRARY_NAMES)
       find_library (
         MATLAB_${_MATLAB_LIB}_LIBRARY
           NAMES "${_MATLAB_LIB}"
           PATHS "${MATLAB_LIB_DIR}"
           HINTS ENV LD_LIBRARY_PATH
           DOC   "MATLAB ${_MATLAB_LIB} link library."
           NO_CMAKE_SYSTEM_PATH
       )
     endforeach ()
 
   endif ()
   # mark variables as advanced
   mark_as_advanced (MATLAB_INCLUDE_DIR)
   foreach (_MATLAB_LIB IN LISTS _MATLAB_LIBRARY_NAMES)
     mark_as_advanced (MATLAB_${_MATLAB_LIB}_LIBRARY)
   endforeach ()
   # list of all libraries
   set (MATLAB_LIBRARY)
   foreach (_MATLAB_LIB IN LISTS _MATLAB_LIBRARY_NAMES)
     if (MATLAB_${_MATLAB_LIB}_LIBRARY)
       list (APPEND MATLAB_LIBRARY "${MATLAB_${_MATLAB_LIB}_LIBRARY}")
     endif ()
   endforeach ()
   # prerequisite libraries
   set (MATLAB_INCLUDES  "${MATLAB_INCLUDE_DIR}")
   set (MATLAB_LIBRARIES "${MATLAB_LIBRARY}")
   # aliases / backwards compatibility
   set (MATLAB_INCLUDE_DIRS "${MATLAB_INCLUDES}")
 
 endif ()
 
 
 # ----------------------------------------------------------------------------
 # handle the QUIETLY and REQUIRED arguments and set *_FOUND to TRUE
 # if all listed variables are found or TRUE
 include (FindPackageHandleStandardArgs)
 
 set (_MATLAB_REQUIRED_VARS)
 
 if (_MATLAB_LIBRARY_NAMES)
   list (APPEND _MATLAB_REQUIRED_VARS MATLAB_INCLUDE_DIR)
   foreach (_MATLAB_LIB IN LISTS _MATLAB_LIBRARY_NAMES)
     list (APPEND _MATLAB_REQUIRED_VARS MATLAB_${_MATLAB_LIB}_LIBRARY)
   endforeach ()
 endif ()
 
 if (_MATLAB_REQUIRED_VARS)
   find_package_handle_standard_args (
     MATLAB
   # MESSAGE
       DEFAULT_MSG
   # VARIABLES
       ${_MATLAB_REQUIRED_VARS}
   )
 else ()
   set (MATLAB_FOUND TRUE)
 endif ()
 
 # ----------------------------------------------------------------------------
 # set MATLAB_DIR
 if (NOT MATLAB_DIR AND MATLAB_INCLUDE_DIR)
   string (REGEX REPLACE "extern/include/?" "" _MATLAB_PREFIX "${MATLAB_INCLUDE_DIR}")
   set (MATLAB_DIR "${_MATLAB_PREFIX}" CACHE PATH "Installation prefix for MATLAB." FORCE)
 endif ()
 
 # ----------------------------------------------------------------------------
 # unset private variables
 unset (_MATLAB_REQUIRED_VARS)
 unset (_MATLAB_EXECUTABLE_NAMES)
 unset (_MATLAB_LIBRARY_NAMES)
 unset (_MATLAB_PREFIX)
 unset (_MATLAB_LIB)
 unset (_MATLAB_EXE)
