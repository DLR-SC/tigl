find_program(ANT_EXECUTABLE
  NAMES ant
  DOC "ant executable file"
  )
mark_as_advanced(ANT_EXECUTABLE)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Ant
                                  REQUIRED_VARS ANT_EXECUTABLE)
