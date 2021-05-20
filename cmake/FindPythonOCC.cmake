# Look for the interface file
FIND_PATH(PythonOCC_SOURCE_DIR
    NAMES src/SWIG_files/wrapper/Standard.i
    PATH_SUFFIXES src/pythonocc-core
)

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(PythonOCC
                                  REQUIRED_VARS PythonOCC_SOURCE_DIR)

IF(PYTHONOCC_FOUND)
    SET(PythonOCC_INCLUDE_DIRS ${PythonOCC_SOURCE_DIR}/src/SWIG_files/wrapper/;${PythonOCC_SOURCE_DIR}/src/SWIG_files/headers)

    if (EXISTS  ${PythonOCC_SOURCE_DIR}/src/SWIG_files/headers/Standard_module.hxx)
        message(STATUS "pythonocc-core 7.x found")
        SET(PYTHONNOCC_LEGACY false)
    else()
        message(STATUS "legacy pythonocc-core 0.1x found")
        SET(PYTHONNOCC_LEGACY true)
    endif()

ENDIF()
