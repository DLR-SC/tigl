option(TIGL_BINDINGS_PYTHON_INTERNAL "Builds the python interface to the internal API(requires SWIG)" OFF)
mark_as_advanced(TIGL_BINDINGS_PYTHON_INTERNAL)

if (POLICY CMP0086)
  cmake_policy(SET CMP0086 OLD)
endif (POLICY CMP0086)

if (POLICY CMP0078)
  cmake_policy(SET CMP0078 OLD)
endif (POLICY CMP0078)

if (TIGL_BINDINGS_PYTHON_INTERNAL)

    if (MSVC)
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /bigobj")
    endif()

    include(tiglmacros)

    find_package(PythonInterp)
    find_package(PythonLibs)
    find_package(SWIG 3.0.11 REQUIRED)
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

    include_directories(${PythonOCC_INCLUDE_DIRS})

    set (MODULES "geometry;occtaddons;tmath;configuration;core;boolean_ops;exports;imports")

    set(LIBRARY_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR})

    add_definitions(-DTIGL_INTERNAL_IMPORTS)
    include_directories($<TARGET_PROPERTY:tigl3_cpp,INTERFACE_INCLUDE_DIRECTORIES>)
    include_directories($<TARGET_PROPERTY:tigl3,INTERFACE_INCLUDE_DIRECTORIES>)

    if (UNIX)
        # find out python linking commands
        execute_process(COMMAND ${PYTHON_EXECUTABLE} -c "from distutils import sysconfig; print(sysconfig.get_config_vars('CFLAGS')[0])" OUTPUT_VARIABLE PYTHON_CFLAGS OUTPUT_STRIP_TRAILING_WHITESPACE )
        execute_process(COMMAND ${PYTHON_EXECUTABLE} -c "from distutils import sysconfig; print(sysconfig.get_config_vars('BLDSHARED')[0].split(' ', 1)[1])" OUTPUT_VARIABLE PYTHON_LDFLAGS OUTPUT_STRIP_TRAILING_WHITESPACE )
    endif(UNIX)

    foreach(MODULE ${MODULES})
        set_source_files_properties(${MODULE}.i PROPERTIES CPLUSPLUS ON)
        set(SWIG_MODULE_${MODULE}_EXTRA_DEPS common.i doc.i)

        swig_add_library(${MODULE} LANGUAGE python SOURCES ${MODULE}.i )
        swig_link_libraries(${MODULE} tigl3 tigl3_cpp)

        if (UNIX)
            set_target_properties(${SWIG_MODULE_${MODULE}_REAL_NAME} PROPERTIES COMPILE_FLAGS ${PYTHON_CFLAGS})
            set_target_properties(${SWIG_MODULE_${MODULE}_REAL_NAME} PROPERTIES LINK_FLAGS ${PYTHON_LDFLAGS})
        else(UNIX)
            swig_link_libraries(${MODULE} ${PYTHON_LIBRARIES})
        endif(UNIX)

        install(TARGETS _${MODULE}
                DESTINATION share/tigl3/python/tigl3
                COMPONENT interfaces
        )

        install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${MODULE}.py
                DESTINATION share/tigl3/python/tigl3
                COMPONENT interfaces
        )

        install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tigl3
                DESTINATION share/tigl3/python
                COMPONENT interfaces
        )

    endforeach(MODULE)


endif(TIGL_BINDINGS_PYTHON_INTERNAL)
