find_library(FREEIMAGE_LIBRARY
  NAMES FreeImage freeimage
  HINTS
    ENV FREEIMAGE_DIR
  PATH_SUFFIXES lib
)



# set the user variables
set(FREEIMAGE_LIBRARIES "${FREEIMAGE_LIBRARY}")

# handle the QUIETLY and REQUIRED arguments and set FREETYPE_FOUND to TRUE if
# all listed variables are TRUE
include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(FreeImageLib
                                  REQUIRED_VARS FREEIMAGE_LIBRARY )

mark_as_advanced(FREEIMAGE_LIBRARY)

if(FreeImageLib_FOUND AND NOT TARGET LibXml2::LibXml2)
   add_library(freeimage::freeimage UNKNOWN IMPORTED)
   set_property(TARGET freeimage::freeimage APPEND PROPERTY IMPORTED_LOCATION "${FREEIMAGE_LIBRARY}")
endif()