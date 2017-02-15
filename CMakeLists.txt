cmake_minimum_required (VERSION 2.8.12)

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
set(TIGL_VERSION_MINOR 2)
set(TIGL_VERSION_PATCH 0)
set(TIGL_VERSION "${TIGL_VERSION_MAJOR}.${TIGL_VERSION_MINOR}.${TIGL_VERSION_PATCH}")

set(TIGL_ADD_INCLUDE_PATH "" CACHE PATH "Additional include path for package search")
set(TIGL_ADD_LIB_PATH "" CACHE PATH     "Additional library path for package search")

set(CMAKE_INCLUDE_PATH "${CMAKE_INCLUDE_PATH}" ${TIGL_ADD_INCLUDE_PATH})
set(CMAKE_LIBRARY_PATH "${CMAKE_LIBRARY_PATH}" ${TIGL_ADD_LIB_PATH})

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

if(NOT DEFINED CMAKE_INSTALL_LIBDIR)
    set(CMAKE_INSTALL_LIBDIR "lib")
endif(NOT DEFINED CMAKE_INSTALL_LIBDIR)


if(NOT DEFINED CMAKE_INSTALL_BINDIR)
    set(CMAKE_INSTALL_BINDIR "bin")
endif(NOT DEFINED CMAKE_INSTALL_BINDIR)

# these settings are required in order to create fully relocatable
# libraries on osx
set(CMAKE_MACOSX_RPATH ON)
set(CMAKE_SKIP_BUILD_RPATH FALSE)
set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)
set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

OPTION(TIGL_NIGHTLY "Creates a nightly build of tigl (includes git sha into tigl version)" OFF)
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
# please don't change the order of libraries as this can cause linking problems
set (OCC_LIBS 
        TKIGES TKSTL TKSTEP 
        TKSTEPAttr TKOffset TKMesh 
        TKSTEP209 TKFillet TKSTEPBase 
        TKBool TKXSBase TKShHealing 
        TKBO TKPrim TKTopAlgo 
        TKGeomAlgo TKBRep TKGeomBase
        TKG3d TKG2d TKMath TKernel
)

set (OCC_VISU_LIBS
	TKV3d TKMesh TKHLR TKService
)

# search OCE. If OCE is not found, search for standard OpenCASCADE
set(CMAKE_PREFIX_PATH "$ENV{CASROOT};${CASROOT};${CMAKE_PREFIX_PATH}")
find_package(OCE 0.15 COMPONENTS ${OCC_LIBS} QUIET)
if(OCE_FOUND)
  set(OpenCASCADE_LIBRARIES ${OCC_LIBS})
  set(OpenCASCADE_INCLUDE_DIR ${OCE_INCLUDE_DIRS})
  # get opencascade version
  IF(EXISTS "${OpenCASCADE_INCLUDE_DIR}/Standard_Version.hxx")
    FILE(STRINGS "${OpenCASCADE_INCLUDE_DIR}/Standard_Version.hxx" occ_version_str REGEX "^#define[\t ]+OCC_VERSION_COMPLETE[\t ]+\".*\"")
    STRING(REGEX REPLACE "^#define[\t ]+OCC_VERSION_COMPLETE[\t ]+\"([^\"]*)\".*" "\\1" OCC_VERSION_STRING "${occ_version_str}")
    UNSET(occ_version_str)
  ENDIF()
  # get directory of shared libs
  get_target_property(TKERNEL_LOCATION TKernel LOCATION)
  get_filename_component(OpenCASCADE_DLL_DIRECTORY ${TKERNEL_LOCATION} PATH)
else(OCE_FOUND)
  message("OCE not found! Searching for OpenCASCADE.")
  find_package(OpenCASCADE 6.6.0 REQUIRED COMPONENTS ${OCC_LIBS})
endif(OCE_FOUND)

# search TiXI
set(TIXI_PATH "" CACHE PATH "TiXI installation prefix")
set(CMAKE_PREFIX_PATH "${TIXI_PATH};${CMAKE_PREFIX_PATH}")
find_package( TIXI 2.2.1 REQUIRED)
get_target_property(TIXI_LIBRARY TIXI LOCATION)
get_filename_component(TIXI_LIB_PATH ${TIXI_LIBRARY} PATH)


find_package( PythonInterp )

OPTION(TIGL_USE_GLOG "Enables advanced logging (requires google glog)" OFF)
if(TIGL_USE_GLOG)
    find_package( GLOG REQUIRED )
    if(NOT GLOG_FOUND)
      message(STATUS "Google GLOG not found. Advanced logging disabled.")
    endif()
endif(TIGL_USE_GLOG)

# check for include files and other functions
INCLUDE(CheckCXXSourceCompiles)
SET(SRC1 "#include <memory>\nint main(){\n std::shared_ptr<int> a(new int)\;\n return 0\;\n}\n")
CHECK_CXX_SOURCE_COMPILES(${SRC1} HAVE_STDSHARED_PTR)

# enable parallel builds in Visual Studio
if (MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
endif()

# enable C++11 on non-windows system (windows uses most recent C++ standard automatically)
if (NOT WIN32)
	set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif (NOT WIN32)

CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/src/tigl_config.h.in ${CMAKE_CURRENT_BINARY_DIR}/src/tigl_config.h)

# code coverage analysis
IF (CMAKE_BUILD_TYPE STREQUAL "Debug")
        OPTION(TIGL_ENABLE_COVERAGE "Enable GCov coverage analysis (defines a 'coverage' target and enforces static build of tigl)" ON)
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
if(WIN32)
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        OPTION(TIGL_USE_VLD "Enable Visual Leak Detector." OFF)
        if(TIGL_USE_VLD)
            find_package( VLD REQUIRED )
            add_definitions(-DHAVE_VLD=1)
            include_directories(${VLD_INCLUDE_DIRS})
        endif(TIGL_USE_VLD)
    endif(CMAKE_BUILD_TYPE STREQUAL "Debug")
endif(WIN32)

if(WIN32)
    # avoid export of oce/occt classes into tigl library
    add_definitions(-DStandard_EXPORT=)
endif(WIN32)

# style checks
add_custom_target(checkstyle)
add_custom_target(checkstylexml)

add_subdirectory(src)
add_subdirectory(TIGLViewer)
add_subdirectory(bindings)
add_subdirectory(examples)


#create gtests, override gtest standard setting
option(TIGL_BUILD_TESTS "Build TIGL Testsuite" OFF)

if(TIGL_BUILD_TESTS)
  enable_testing()
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
