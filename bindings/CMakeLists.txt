
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

    # distutils.sysconfig was removed in Python 3.12 (PEP 632). Ask sysconfig for the
    # site-packages path as if the prefix were empty, giving a path relative to
    # CMAKE_INSTALL_PREFIX (which isn't always this interpreter's own sys.prefix).
    execute_process(
        COMMAND "${Python3_EXECUTABLE}" -c "if True:
        import sysconfig, os
        p = sysconfig.get_path('platlib', vars={'base': '', 'platbase': ''})
        print(p.lstrip(os.sep))"
        OUTPUT_VARIABLE PYTHON_SITE_PACKAGES
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_VARIABLE PYTHON_SITE_PACKAGES_ERROR
    )

    if (PYTHON_SITE_PACKAGES_ERROR OR NOT PYTHON_SITE_PACKAGES)
        set(PYTHON_SITE_PACKAGES share/tigl3/python)
        message(WARNING "Python site-packages directory could not be found (missing setuptools, or "
                         "sysconfig otherwise failed). Falling back to: ${PYTHON_SITE_PACKAGES}")
    else()
        cmake_path(SET PYTHON_SITE_PACKAGES NORMALIZE "${PYTHON_SITE_PACKAGES}")
        # Defense in depth: the template-based query above should always yield a plain relative
        # subpath, but if some install scheme ever returns something else, don't silently try to
        # install far outside the intended prefix (or elsewhere entirely) - fall back instead.
        cmake_path(IS_ABSOLUTE PYTHON_SITE_PACKAGES PYTHON_SITE_PACKAGES_IS_ABSOLUTE)
        if (PYTHON_SITE_PACKAGES_IS_ABSOLUTE OR PYTHON_SITE_PACKAGES MATCHES "^\\.\\.")
            set(PYTHON_SITE_PACKAGES share/tigl3/python)
            message(WARNING "Computed Python site-packages directory does not resolve to a proper "
                             "subdirectory of the install prefix. Falling back to: ${PYTHON_SITE_PACKAGES}")
        else()
            message(STATUS "Python site-packages: ${PYTHON_SITE_PACKAGES}")
        endif()
    endif()

    add_subdirectory(python)
    add_subdirectory(python_internal)
endif(TIGL_BINDINGS_PYTHON)

add_subdirectory(matlab)
add_subdirectory(java)
