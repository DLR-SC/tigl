# Look for the interface file
FIND_PATH(PythonOCC_SOURCE_DIR
    NAMES src/SWIG_files/wrapper/Standard.i
)

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(PythonOCC
                                  REQUIRED_VARS PythonOCC_SOURCE_DIR)

IF(PYTHONOCC_FOUND)
    SET(PythonOCC_INCLUDE_DIRS ${PythonOCC_SOURCE_DIR}/src/SWIG_files/wrapper/)
ENDIF()
