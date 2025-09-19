
option(TIGL_BINDINGS_PYTHON "Build the python bindings of tigl (requires python >= 2.5)" ON)

if (TIGL_BINDINGS_PYTHON)

    option(TIGL_INSTALL_TO_PYTHON_SITE_PACKAGES "Install the Python bindings directly into the python's site package directory" OFF)

    # Ensure the correct Python version is picked up by specifying the Python_EXECUTABLE
    if (DEFINED ENV{CONDA_PREFIX})
        if (WIN32)
            set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} "$ENV{CONDA_PREFIX}/Library")
            # Helps to find PythonOCC automatically
            set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} "$ENV{CONDA_PREFIX}/Library/src/pythonocc-core")
        else()
            set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} "$ENV{CONDA_PREFIX}")
            # Helps to find PythonOCC automatically
            set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} "$ENV{CONDA_PREFIX}/src/pythonocc-core")
        endif()
    endif()

    find_package(Python3 COMPONENTS Interpreter Development REQUIRED)
    message(STATUS "Python3 interpreter:" ${Python3_EXECUTABLE})
    message(STATUS "Python include directory: ${Python3_INCLUDE_DIR}")
    message(STATUS "Python library release: ${Python3_LIBRARY_RELEASE}")

    if (TIGL_INSTALL_TO_PYTHON_SITE_PACKAGES)
        # this requires setuptools to be installed
        #TODO error or fallback if this fails!
        execute_process(
            COMMAND "${PYTHON_EXECUTABLE}" -c "if True:
            from distutils import sysconfig as sc
            print(sc.get_python_lib(prefix='', plat_specific=True))"
            OUTPUT_VARIABLE PYTHON_SITE_PACKAGES
            OUTPUT_STRIP_TRAILING_WHITESPACE
        )
    else()
        set(PYTHON_SITE_PACKAGES share/tigl3/python)
    endif()
    message(STATUS "Python site-packages: ${PYTHON_SITE_PACKAGES}")

    add_subdirectory(python)
    add_subdirectory(python_internal)
endif(TIGL_BINDINGS_PYTHON)

add_subdirectory(matlab)
add_subdirectory(java)