# TIGL Fortran interface
# @author: Martin Siggel

set(TIGL_F_SRC tigl_fortran.cpp)
set(TIGL_F_HDR tigl_fortran.h)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../src ${CMAKE_CURRENT_SOURCE_DIR}/../../src/api ${TIXI_INCLUDE_DIRS})

#find out the fortran compiler
include(CMakeDetermineFortranCompiler)

if(CMAKE_Fortran_COMPILER STREQUAL CMAKE_Fortran_COMPILER-NOTFOUND)
	message(STATUS "No Fortran Compiler found, please specify for which compiler the interface should be built")
else()
	if(${CMAKE_Fortran_COMPILER} MATCHES "gfortran")
		message(STATUS "Generating fortran interface for gfortran naming scheme.")
		add_definitions(-DGFORTRAN)
	elseif(${CMAKE_Fortran_COMPILER} MATCHES "g77")
		message(STATUS "Generating fortran interface for g77 naming scheme.")
		add_definitions(-DG77)
	elseif(${CMAKE_Fortran_COMPILER} MATCHES "f90_irix")
		message(STATUS "Generating fortran interface for f90/irix naming scheme.")
		add_definitions(-DF90_IRIX)
	elseif(${CMAKE_Fortran_COMPILER} MATCHES "f90")
		message(STATUS "Generating fortran interface for f90 naming scheme.")
		add_definitions(-DGFORTRAN)
	elseif(${CMAKE_Fortran_COMPILER} MATCHES "ifort")
		message(STATUS "Generating fortran interface for intel-fortran (ifort) naming scheme.")
		add_definitions(-DIFORT_WIN32)
	endif()

	add_library(tigl3-f90 STATIC ${TIGL_F_SRC} ${TIGL_F_HDR})
	set_target_properties(tigl3-f90 PROPERTIES VERSION ${TIGL_VERSION})
	install (TARGETS tigl3-f90 
			DESTINATION ${CMAKE_INSTALL_LIBDIR}
			COMPONENT interfaces)
	install (FILES tigl_fortran.h 
			DESTINATION include/tigl3
			COMPONENT interfaces)
	
endif()

