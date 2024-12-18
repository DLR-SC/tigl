# Set a default build type if none was specified
if(NOT DEFINED CMAKE_BUILD_TYPE)
  message(STATUS "Setting build type to 'Release' as none was specified.")
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build." FORCE)
  # Set the possible values of build type for cmake-gui
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release"
    "MinSizeRel" "RelWithDebInfo")
endif()

cmake_minimum_required (VERSION 3.11.0)

project (TIGL VERSION 3.4.0)
set(TIGL_VERSION 3.4.0)

# enable C++17 support
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

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

# convert path to absolute (required for some scripts)
if (NOT IS_ABSOLUTE ${CMAKE_INSTALL_PREFIX})
   set (CMAKE_INSTALL_PREFIX ${PROJECT_BINARY_DIR}/${CMAKE_INSTALL_PREFIX})
endif()

option(TIGL_BINDINGS_INSTALL_CPP "Install TiGL's CPP bindings" OFF)

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

include(UseOpenCASCADE)

# search TiXI
set(TIXI_PATH "" CACHE PATH "TiXI installation prefix")
set(CMAKE_PREFIX_PATH "${TIXI_PATH};${CMAKE_PREFIX_PATH}")
find_package( tixi3 3.0.3 REQUIRED CONFIG)


find_package( PythonInterp )

OPTION(TIGL_USE_GLOG "Enables advanced logging (requires google glog)" OFF)
if(TIGL_USE_GLOG)
    find_package( GLOG REQUIRED )
    if(NOT GLOG_FOUND)
      message(STATUS "Google GLOG not found. Advanced logging disabled.")
    endif()
endif(TIGL_USE_GLOG)

# enable parallel builds in Visual Studio
if (MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
endif()

# check features
include(CheckCXXSourceCompiles)
set(src_make_unqiue "#include <memory>\nint main(){\n std::make_unique<int>(5)\;\n return 0\;\n}\n")
CHECK_CXX_SOURCE_COMPILES(${src_make_unqiue} HAVE_STDMAKE_UNIQUE)

# code coverage analysis
IF (CMAKE_BUILD_TYPE STREQUAL "Debug")
        # enable extensive debug output
        OPTION(DEBUG_EXTENSIVE "Switch on extensive debug output" OFF)
        OPTION(TIGL_ENABLE_COVERAGE "Enable GCov coverage analysis (defines a 'coverage' target and enforces static build of tigl)" OFF)
        IF(TIGL_ENABLE_COVERAGE)
                MESSAGE(STATUS "Coverage enabled")
                INCLUDE(CodeCoverage)
                SETUP_TARGET_FOR_COVERAGE(coverage-unittests TiGL-unittests tests/unittests coverageReport-unit $ENV{COVERAGE_ARGS})
                SETUP_TARGET_FOR_COVERAGE(coverage-integrationtests TiGL-integrationtests tests/integrationtests coverageReport-integration $ENV{COVERAGE_ARGS})
                SETUP_TARGET_FOR_COVERAGE_COBERTURA(coverage-cobertura TiGL-unittests coverage $ENV{COVERAGE_ARGS})
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
    
endif(WIN32)

add_subdirectory(thirdparty)

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
  mark_as_advanced(gtest_force_shared_crt gtest_build_tests gtest_build_samples gtest_disable_pthreads)
  add_subdirectory ("thirdparty/googletest" EXCLUDE_FROM_ALL)
  add_subdirectory(tests)
endif(TIGL_BUILD_TESTS)

include(createDoc)


set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Martin Siggel") #required for debian/ubuntu
set(CPACK_PACKAGE_VENDOR "www.dlr.de/sc")
if(TIGL_NIGHTLY)
  string(SUBSTRING ${TIGL_REVISION} 0 8 TIGL_REV_SHORT)
  set(CPACK_PACKAGE_VERSION ${TIGL_VERSION}-r${TIGL_REV_SHORT})
  set(CPACK_PACKAGE_VERSION_PATCH ${TIGL_VERSION_PATCH}-r${TIGL_REV_SHORT})
else()
  set(CPACK_PACKAGE_VERSION ${TIGL_VERSION})
  set(CPACK_PACKAGE_VERSION_PATCH ${TIGL_VERSION_PATCH})
endif()
set(CPACK_PACKAGE_VERSION_MAJOR ${TIGL_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${TIGL_VERSION_MINOR})

set(CPACK_RESOURCE_FILE_LICENSE ${PROJECT_SOURCE_DIR}/LICENSE.txt)
set(CPACK_PACKAGE_INSTALL_REGISTRY_KEY "TIGL") 

set(CPACK_NSIS_MUI_ICON ${PROJECT_SOURCE_DIR}/TIGLViewer/TIGLViewer.ico)
set(CPACK_NSIS_MUI_UNIICON ${PROJECT_SOURCE_DIR}/TIGLViewer/TIGLViewer.ico)

# set installer icon
if(WIN32)
    set(CPACK_PACKAGE_ICON  ${PROJECT_SOURCE_DIR}/TIGLViewer/gfx\\\\TIGLViewerNSIS.bmp)
elseif(APPLE)
    set(CPACK_PACKAGE_ICON  ${PROJECT_SOURCE_DIR}/TIGLViewer/gfx/TiGL-Viewer3.icns)
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

if (APPLE)
  set(CPACK_PACKAGE_EXECUTABLES
    "TiGL-Viewer3" "TiGL Viewer 3"
  )
else(APPLE)
  set(CPACK_PACKAGE_EXECUTABLES
    "tiglviewer-3" "TiGL Viewer 3"
  )
endif(APPLE)

include(CPack)
cpack_add_component(viewer DISPLAY_NAME "TiGL Viewer + 3rd Party DLLs")
cpack_add_component(headers DISPLAY_NAME "Headers")
cpack_add_component(cpp_bindings DISPLAY_NAME "Internal C++ Bindings")
cpack_add_component(interfaces DISPLAY_NAME "Interfaces/Bindings")
cpack_add_component(docu DISPLAY_NAME "Documentation")
