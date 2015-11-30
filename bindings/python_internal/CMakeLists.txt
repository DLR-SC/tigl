option(TIGL_PYTHON_INTERNAL "Builds the python interface to the internal API(requires SWIG)" OFF)

if (TIGL_PYTHON_INTERNAL)
    include(tiglmacros)

    find_package(PythonInterp)
    find_package(PythonLibs)
    find_package(SWIG REQUIRED)
    find_package(PythonOCC REQUIRED)

    include(${SWIG_USE_FILE})

    include_directories(${PYTHON_INCLUDE_PATH})

    # add all tigl directories to include path
    set(TIGL_INCLUDES ${PROJECT_SOURCE_DIR}/src/)
    subdirlist(SUBDIRS ${PROJECT_SOURCE_DIR}/src/)
    foreach(subdir ${SUBDIRS})
        set(TIGL_INCLUDES ${TIGL_INCLUDES} ${PROJECT_SOURCE_DIR}/src/${subdir})
    endforeach()
    include_directories(${TIGL_INCLUDES} ${PROJECT_BINARY_DIR}/src ${TIXI_INCLUDE_DIRS})

    include_directories(${OpenCASCADE_INCLUDE_DIR} ${PythonOCC_INCLUDE_DIRS})

    include_directories(${PROJECT_SOURCE_DIR}/thirdparty/boost_1_55_0)

    set (MODULES "geometry;occtaddons;tmath;configuration;core;boolean_ops;exports;imports")

    set(LIBRARY_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR})

    if(WIN32)
       set(EXTENSION "pyd")
    else(WIN32)
        set(EXTENSION "so")
    endif(WIN32)

    foreach(MODULE ${MODULES})
        set_source_files_properties(${MODULE}.i PROPERTIES CPLUSPLUS ON)
        set(SWIG_MODULE_${MODULE}_EXTRA_DEPS common.i )

        swig_add_module(${MODULE} python ${MODULE}.i )
        swig_link_libraries(${MODULE} ${PYTHON_LIBRARIES} TIGL)

        install(FILES ${CMAKE_CURRENT_BINARY_DIR}/_${MODULE}.${EXTENSION}
                DESTINATION share/tigl/python/internal
                COMPONENT interfaces
        )

        install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${MODULE}.py
                DESTINATION share/tigl/python/internal
                COMPONENT interfaces
        )

    endforeach(MODULE)


endif(TIGL_PYTHON_INTERNAL)
