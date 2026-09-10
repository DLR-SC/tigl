
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
    # locate site-packages here.
    #
    # PYTHON_SITE_PACKAGES needs to be a path *relative to CMAKE_INSTALL_PREFIX*, which is NOT
    # necessarily this interpreter's own sys.prefix: e.g. pixi's "python-internal" environment
    # sets CMAKE_INSTALL_PREFIX to $CONDA_PREFIX (so it coincides with sys.prefix, installing
    # straight into the live environment), but pixi's "default" environment sets it to a local
    # build/install directory instead, to produce a self-contained, relocatable install tree -
    # deliberately independent of where the build-time Python interpreter itself lives.
    #
    # Because of that, the right query is "what does a site-packages subpath look like under any
    # prefix" (a portable template), not "what is my absolute site-packages, diffed against
    # sys.prefix" - the latter only reconstructs correctly when CMAKE_INSTALL_PREFIX happens to
    # equal sys.prefix, and otherwise surfaces install-scheme quirks as a badly broken path (this
    # was tried and failed exactly this way on one platform/Python combination in CI, producing a
    # long, escaping "../../.." relative path). Get the template directly instead, by asking
    # sysconfig to resolve the path as if the prefix were empty, and stripping the resulting
    # leading path separator (os.sep, so no separator literal has to survive CMake's own string
    # escaping).
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
