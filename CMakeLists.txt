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
set(TIGL_VERSION_PATCH 1)
set(TIGL_VERSION "${TIGL_VERSION_MAJOR}.${TIGL_VERSION_MINOR}.${TIGL_VERSION_PATCH}")

set(ADD_INCLUDE_PATH "" CACHE PATH "Additional include path for package search")
set(ADD_LIB_PATH "" CACHE PATH     "Additional library path for package search")

set(CMAKE_INCLUDE_PATH "${CMAKE_INCLUDE_PATH}" ${ADD_INCLUDE_PATH})
set(CMAKE_LIBRARY_PATH "${CMAKE_LIBRARY_PATH}" ${ADD_LIB_PATH})

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/contrib)
include(GNUInstallDirs)

set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR})

# required opencascade libraries
set (OCC_LIBS 
	PTKernel TKAdvTools TKBin TKBinL TKBinXCAF
	TKBO TKBool	TKBRep TKCAF TKCDF 
	TKernel TKFeat TKFillet TKG2d
	TKG3d TKGeomAlgo TKGeomBase TKHLR TKIGES
	TKLCAF TKMath TKMesh TKMeshVS TKOffset
	TKOpenGl TKPCAF TKPLCAF TKPrim TKPShape
	TKService TKShapeSchema TKShHealing TKStdLSchema
	TKStdSchema TKSTEP TKSTEP209 TKSTEPAttr TKSTEPBase
	TKSTL TKTopAlgo TKV2d TKV3d 
	TKVRML TKXCAF TKXCAFSchema TKXDEIGES
	TKXDESTEP TKXml TKXmlL TKXmlXCAF TKXSBase 
)

find_package( OpenCASCADE REQUIRED COMPONENTS ${OCC_LIBS})
find_package( TIXI REQUIRED)

add_subdirectory(src)
add_subdirectory(TIGLViewer)
add_subdirectory(tests)
include(createDoc)

set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Martin Siggel") #required for debian/ubuntu
set(CPACK_PACKAGE_VENDOR "www.dlr.de/sc")
set(CPACK_PACKAGE_VERSION ${TIGL_VERSION})
set(CPACK_PACKAGE_VERSION_MAJOR ${TIGL_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${TIGL_VERSION_MINOR})
set(CPACL_PACKAGE_VERSION_PATCH ${TIGL_VERSION_PATCH})
set(CPACK_RESOURCE_FILE_LICENSE ${PROJECT_SOURCE_DIR}/LICENSE.txt)
set(CPACK_PACKAGE_INSTALL_REGISTRY_KEY "TIGL") 

# set path variable for installer
set(CPACK_NSIS_MODIFY_PATH ON)
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
  set(CPACK_NSIS_INSTALL_ROOT "$PROGRAMFILES64")
else()
  set(CPACK_NSIS_INSTALL_ROOT "$PROGRAMFILES")
endif()

set(CPACK_PACKAGE_EXECUTABLES 
	"TIGLViewer" "TIGL Viewer"
)

include(CPack)
cpack_add_component(viewer DISPLAY_NAME "TIGL Viewer + 3rd Party DLLs")
cpack_add_component(headers DISPLAY_NAME "Headers")
#cpack_add_component(interfaces DISPLAY_NAME "Interfaces/Bindings")
cpack_add_component(docu DISPLAY_NAME "Documentation")
