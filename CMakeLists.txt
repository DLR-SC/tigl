cmake_minimum_required (VERSION 2.6)

# Set a default build type if none was specified
if(NOT DEFINED CMAKE_BUILD_TYPE)
  message(STATUS "Setting build type to 'Release' as none was specified.")
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build." FORCE)
  # Set the possible values of build type for cmake-gui
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release"
    "MinSizeRel" "RelWithDebInfo")
endif()

project (TIGL)
set(TIGL_VERSION_MAJOR 2)
set(TIGL_VERSION_MINOR 0)
set(TIGL_VERSION_PATCH 4+)
set(TIGL_VERSION "${TIGL_VERSION_MAJOR}.${TIGL_VERSION_MINOR}.${TIGL_VERSION_PATCH}")

set(TIGL_ADD_INCLUDE_PATH "" CACHE PATH "Additional include path for package search")
set(TIGL_ADD_LIB_PATH "" CACHE PATH     "Additional library path for package search")

set(CMAKE_INCLUDE_PATH "${CMAKE_INCLUDE_PATH}" ${TIGL_ADD_INCLUDE_PATH})
set(CMAKE_LIBRARY_PATH "${CMAKE_LIBRARY_PATH}" ${TIGL_ADD_LIB_PATH})

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/contrib)
include(GNUInstallDirs)

OPTION(TIGL_NIGHTLY "Creates a nightly build of tigl (includes git sha into tigl version)s" OFF)
mark_as_advanced(TIGL_NIGHTLY)
if(TIGL_NIGHTLY)
    message(STATUS "Nightly build enabled")

    # get git revision for daily builds
    include(GetGitRevisionDescription)
    get_git_head_revision(REFSPEC HASHVAR)

    if(NOT ${HASHVAR} STREQUAL "GITDIR-NOTFOUND")
        set(TIGL_REVISION ${HASHVAR})
    endif()
endif(TIGL_NIGHTLY)

set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR})

# required opencascade libraries
set (OCC_LIBS 
	FWOSPlugin
	TKBO TKBool	TKBRep  
	TKernel  TKG2d TKXDEIGES TKCAF
	TKG3d TKGeomAlgo TKGeomBase  TKIGES
	TKMath TKMesh TKOffset TKPCAF
	TKShHealing TKLCAF TKTopAlgo TKXCAF
	TKSTL TKTopAlgo  TKStdSchema
	TKXSBase TKStdLSchema  TKSTEPBase TKXDESTEP TKSTEP
)

find_package( OpenCASCADE REQUIRED COMPONENTS ${OCC_LIBS})
find_package( TIXI 2.0.4 REQUIRED)
find_package( GLOG REQUIRED)

CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/src/tigl_config.h.in ${CMAKE_CURRENT_BINARY_DIR}/src/tigl_config.h)

# code coverage analysis
IF (CMAKE_BUILD_TYPE STREQUAL "Debug")
	OPTION(TIGL_ENABLE_COVERAGE "Enable GCov coverage analysis (defines a 'coverage' target and enforces static build of tigl)" OFF)
        IF(TIGL_ENABLE_COVERAGE)
		
                MESSAGE(STATUS "Coverage enabled")
                INCLUDE(CodeCoverage)
                SETUP_TARGET_FOR_COVERAGE(coverage ${PROJECT_NAME}-unittests coverageReport)
                SETUP_TARGET_FOR_COVERAGE_COBERTURA(coverage-cobertura ${PROJECT_NAME}-unittests coverage)
        ELSE()
                MESSAGE(STATUS "Coverage disabled")
        ENDIF()
ENDIF()

# visual leak detector, useful for debugging under windows
find_package( VLD )
if(VLD_FOUND)
	set(TIGL_USE_VLD False CACHE BOOL "Enable Visual Leak Detector.")
	if(TIGL_USE_VLD)
		add_definitions(-DHAVE_VLD=1)
		include_directories(${VLD_INCLUDE_DIRS})
	endif(TIGL_USE_VLD)
endif()


add_subdirectory(src)
add_subdirectory(TIGLViewer)
add_subdirectory(bindings)


#create gtests, override gtest standard setting
option(TIGL_BUILD_TESTS "Build TIGL Testsuite" OFF)

if(TIGL_BUILD_TESTS)
  option(gtest_force_shared_crt "" ON)
  mark_as_advanced(gtest_force_shared_crt)
  set (gtest_SOURCE_DIR "thirdparty/gtest-1.6.0")
  add_subdirectory (${gtest_SOURCE_DIR})
  add_subdirectory(tests)
endif(TIGL_BUILD_TESTS)

include(createDoc)


set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Martin Siggel") #required for debian/ubuntu
set(CPACK_PACKAGE_VENDOR "www.dlr.de/sc")
set(CPACK_PACKAGE_VERSION ${TIGL_VERSION})
set(CPACK_PACKAGE_VERSION_MAJOR ${TIGL_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${TIGL_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${TIGL_VERSION_PATCH})
set(CPACK_RESOURCE_FILE_LICENSE ${PROJECT_SOURCE_DIR}/LICENSE.txt)
set(CPACK_PACKAGE_INSTALL_REGISTRY_KEY "TIGL") 

set(CPACK_NSIS_MUI_ICON ${PROJECT_SOURCE_DIR}/TIGLViewer/TIGLViewer.ico)
set(CPACK_NSIS_MUI_UNIICON ${PROJECT_SOURCE_DIR}/TIGLViewer/TIGLViewer.ico)

# set installer icon
if(WIN32)
    set(CPACK_PACKAGE_ICON  ${PROJECT_SOURCE_DIR}/TIGLViewer/gfx\\\\TIGLViewerNSIS.bmp)
elseif(APPLE)
    set(CPACK_PACKAGE_ICON  ${PROJECT_SOURCE_DIR}/TIGLViewer/gfx/TIGLViewer.icns)
endif()

# set generators
if(CPACK_GENERATOR)	
    #already set
elseif(APPLE)
    set(CPACK_GENERATOR DragNDrop)
elseif(WIN32)
    set(CPACK_GENERATOR "NSIS;ZIP")
else()
    set(CPACK_GENERATOR TGZ)
endif()

# set path variable for installer
set(CPACK_NSIS_MODIFY_PATH ON)
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
  set(CPACK_NSIS_INSTALL_ROOT "$PROGRAMFILES64")
  set(CPACK_CUSTOM_INITIAL_DEFINITIONS "!define CPACK_REQUIRIRE_64BIT")
else()
  set(CPACK_NSIS_INSTALL_ROOT "$PROGRAMFILES")
endif()

set(CPACK_PACKAGE_EXECUTABLES 
	"TIGLViewer" "TIGL Viewer"
)

include(CPack)
cpack_add_component(viewer DISPLAY_NAME "TIGL Viewer + 3rd Party DLLs")
cpack_add_component(headers DISPLAY_NAME "Headers")
cpack_add_component(interfaces DISPLAY_NAME "Interfaces/Bindings")
cpack_add_component(docu DISPLAY_NAME "Documentation")
