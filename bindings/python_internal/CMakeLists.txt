option(TIGL_BINDINGS_PYTHON_INTERNAL "Builds the python interface to the internal API(requires SWIG)" OFF)
mark_as_advanced(TIGL_BINDINGS_PYTHON_INTERNAL)

if (TIGL_BINDINGS_PYTHON_INTERNAL)
    include(tiglmacros)

    find_package(PythonInterp)
    find_package(PythonLibs)
    find_package(SWIG REQUIRED)
    find_package(PythonOCC REQUIRED)
    find_package(Doxygen 1.8.0)

    if(NOT DOXYGEN_FOUND)
        add_custom_command(
            OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/doc.i
            COMMAND ${CMAKE_COMMAND}
            ARGS -E touch ${CMAKE_CURRENT_BINARY_DIR}/doc.i
        )
    else()
        # we build the swig documentation as follows
        # 1) Run doxygen to create an xml output
        # 2) Run doxy2swig to create doc.i from the xml output
        # 3) Include doc.i in the swig wrapper files

        # create the Doxyfile
        configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY)

        # create the xml output with doxygen
        add_custom_command(
            OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/doc/xml/index.xml
            DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
            COMMAND ${DOXYGEN_EXECUTABLE}
            ARGS ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
        )

        # create the doc.i with doxy2swig
        add_custom_command(
            OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/doc.i
            DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/doc/xml/index.xml
            COMMAND ${PYTHON_EXECUTABLE}
            ARGS ${PROJECT_SOURCE_DIR}/thirdparty/doxy2swig/doxy2swig.py ${CMAKE_CURRENT_BINARY_DIR}/doc/xml/index.xml ${CMAKE_CURRENT_BINARY_DIR}/doc.i
        )
    endif()


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

    include_directories(${PROJECT_SOURCE_DIR}/thirdparty/boost_1_63_0)

    set (MODULES "geometry;occtaddons;tmath;configuration;core;boolean_ops;exports;imports")

    set(LIBRARY_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR})

    if(WIN32)
       set(EXTENSION "pyd")
    else(WIN32)
        set(EXTENSION "so")
    endif(WIN32)

    foreach(MODULE ${MODULES})
        set_source_files_properties(${MODULE}.i PROPERTIES CPLUSPLUS ON)
        set(SWIG_MODULE_${MODULE}_EXTRA_DEPS common.i doc.i)

        swig_add_module(${MODULE} python ${MODULE}.i )
        swig_link_libraries(${MODULE} ${PYTHON_LIBRARIES} TIGL)

        install(TARGETS _${MODULE}
                DESTINATION share/tigl/python/internal
                COMPONENT interfaces
        )

        install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${MODULE}.py
                DESTINATION share/tigl/python/internal
                COMPONENT interfaces
        )

    endforeach(MODULE)


endif(TIGL_BINDINGS_PYTHON_INTERNAL)
