# The pythonocc-core SWIG interface files (src/SWIG_files/...) are not shipped by the
# conda-forge pythonocc-core package (only the compiled OCC Python module is). Until that is
# addressed upstream (https://github.com/conda-forge/pythonocc-feedstock), TiGL vendors the
# matching pythonocc-core release as a git submodule at thirdparty/pythonocc-core -- keep its
# pinned tag in sync with the pythonocc-core/occt version in pixi.toml.
FIND_PATH(PythonOCC_SOURCE_DIR
    NAMES src/SWIG_files/wrapper/Standard.i
    PATHS ${CMAKE_SOURCE_DIR}/thirdparty/pythonocc-core
    NO_DEFAULT_PATH
)

IF(NOT PythonOCC_SOURCE_DIR)
    # Fall back to a conda-provided source tree, if any (e.g. a non-conda-forge channel).
    FIND_PATH(PythonOCC_SOURCE_DIR
        NAMES src/SWIG_files/wrapper/Standard.i
        PATH_SUFFIXES src/pythonocc-core
    )
ENDIF()

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
