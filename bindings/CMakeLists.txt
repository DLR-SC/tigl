
option(TIGL_BINDINGS_PYTHON "Build the python bindings of tigl (requires python >= 2.5)" ON)

if (TIGL_BINDINGS_PYTHON)

    # Ensure the correct Python version is picked up by specifying the Python_EXECUTABLE
    if (DEFINED ENV{CONDA_PREFIX})
        if (WIN32)
            set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} "$ENV{CONDA_PREFIX}/Library")
        else()
            set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} "$ENV{CONDA_PREFIX}")
        endif()
    endif()

    find_package(Python3 COMPONENTS Interpreter Development REQUIRED)
    message(STATUS "Python3 interpreter:" ${Python3_EXECUTABLE})
    message(STATUS "Python include directory: ${Python3_INCLUDE_DIR}")
    message(STATUS "Python library release: ${Python3_LIBRARY_RELEASE}")

    # distutils.sysconfig was removed in Python 3.12 (PEP 632), so it can no longer be used to
    # locate site-packages here. Use the stdlib sysconfig module instead, forcing both of its
    # install-scheme template variables to sys.prefix so the result can be made relative to it
    # (matching the prefix='' relative-path behavior the old distutils.get_python_lib() call had).
    execute_process(
        COMMAND "${Python3_EXECUTABLE}" -c "if True:
        import sysconfig, sys, os
        p = sysconfig.get_path('platlib', vars={'base': sys.prefix, 'platbase': sys.prefix})
        print(os.path.relpath(p, sys.prefix))"
        OUTPUT_VARIABLE PYTHON_SITE_PACKAGES
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_VARIABLE PYTHON_SITE_PACKAGES_ERROR
    )

    if (PYTHON_SITE_PACKAGES_ERROR)
        set(PYTHON_SITE_PACKAGES share/tigl3/python)
        message(WARNING "Python site-packages directory could not be found. Falling back to: ${PYTHON_SITE_PACKAGES}")
    else()
        cmake_path(SET PYTHON_SITE_PACKAGES NORMALIZE "${PYTHON_SITE_PACKAGES}")
        message(STATUS "Python site-packages: ${PYTHON_SITE_PACKAGES}")
    endif()

    add_subdirectory(python)
    add_subdirectory(python_internal)
endif(TIGL_BINDINGS_PYTHON)

add_subdirectory(matlab)
add_subdirectory(java)
