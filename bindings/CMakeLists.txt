
option(TIGL_BINDINGS_PYTHON "Build the python bindings of tigl (requires python >= 2.5)" ON)

if (TIGL_BINDINGS_PYTHON)

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

    # this requires setuptools to be installed
    execute_process(
        COMMAND "${Python3_EXECUTABLE}" -c "if True:
        from distutils import sysconfig as sc
        print(sc.get_python_lib(prefix='', plat_specific=True))"
        OUTPUT_VARIABLE PYTHON_SITE_PACKAGES
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_VARIABLE PYTHON_SITE_PACKAGES_ERROR
    )

    if (PYTHON_SITE_PACKAGES_ERROR)
        set(PYTHON_SITE_PACKAGES share/tigl3/python)
        message(WARNING "Python site-packages directory could not be found (missing setuptools?). Falling back to: ${PYTHON_SITE_PACKAGES}")
    else()
        message(STATUS "Python site-packages: ${PYTHON_SITE_PACKAGES}")
    endif()

    add_subdirectory(python)
    add_subdirectory(python_internal)
endif(TIGL_BINDINGS_PYTHON)

add_subdirectory(matlab)
add_subdirectory(java)
