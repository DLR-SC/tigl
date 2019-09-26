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
OPTION(OpenCASCADE_DONT_SEARCH_OCE "Disabled searching for OCE" OFF)
if(NOT OpenCASCADE_DONT_SEARCH_OCE)
  find_package(OCE 0.15 COMPONENTS ${OCC_LIBS} QUIET)
endif()

if(OCE_FOUND)
  set(OpenCASCADE_LIBRARIES ${OCC_LIBS})
  set(OpenCASCADE_INCLUDE_DIR ${OCE_INCLUDE_DIRS})

  # set shaders directory
  if (NOT ${OCE_VERSION} VERSION_LESS "0.17")
      FIND_PATH(OpenCASCADE_SHADER_DIRECTORY
                NAMES PhongShading.fs
                PATH_SUFFIXES share/oce/src/Shaders share/oce-${OCE_VERSION}/src/Shaders
                HINTS ${CASROOT} ${OCE_INCLUDE_DIRS}/../../
      )
  endif()


  # get opencascade version
  IF(EXISTS "${OpenCASCADE_INCLUDE_DIR}/Standard_Version.hxx")
    FILE(STRINGS "${OpenCASCADE_INCLUDE_DIR}/Standard_Version.hxx" occ_version_str REGEX "^#define[\t ]+OCC_VERSION_COMPLETE[\t ]+\".*\"")
    STRING(REGEX REPLACE "^#define[\t ]+OCC_VERSION_COMPLETE[\t ]+\"([^\"]*)\".*" "\\1" OCC_VERSION_STRING "${occ_version_str}")
    UNSET(occ_version_str)
  ENDIF()
  # get directory of shared libs
  get_target_property(TKERNEL_LOCATION TKernel LOCATION)
  get_filename_component(OpenCASCADE_DLL_DIRECTORY ${TKERNEL_LOCATION} PATH)
  
  option(OCE_STATIC_LIBS "Should be checked, if static OCE libs are linked" OFF)
else(OCE_FOUND)
  message("OCE not found! Searching for OpenCASCADE.")
  find_package(OpenCASCADE 6.6.0 REQUIRED COMPONENTS ${OCC_LIBS})
  option(OpenCASCADE_STATIC_LIBS "Should be checked, if static OpenCASCADE libs are linked" OFF)
endif(OCE_FOUND)

set_property(TARGET TKernel APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS "Standard_EXPORT=")
set_target_properties(TKernel PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${OpenCASCADE_INCLUDE_DIR}")

if (OCE_STATIC_LIBS OR OpenCASCADE_STATIC_LIBS)
    target_compile_definitions(TKernel INTERFACE HAVE_NO_DLL)
endif()

include(CheckCXXSourceCompiles)

# check if opencascade has patched coons feature
SET(src_patched_occ "
#include <GeomFill_FillingStyle.hxx>
void test(){
  GeomFill_FillingStyle style = GeomFill_CoonsC2Style\;
}\n")

set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
set(CMAKE_REQUIRED_INCLUDES ${OpenCASCADE_INCLUDE_DIR})
CHECK_CXX_SOURCE_COMPILES(${src_patched_occ} HAVE_OCE_COONS_PATCHED)
unset(CMAKE_TRY_COMPILE_TARGET_TYPE)
unset(CMAKE_REQUIRED_INCLUDES)
