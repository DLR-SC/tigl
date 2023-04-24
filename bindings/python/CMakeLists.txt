option(TIGL_BINDINGS_PYTHON "Build the python bindings of tigl (requires python >= 2.5)" ON)

if (TIGL_BINDINGS_PYTHON)

find_package(PythonInterp)

# work around wrong python detection script of cmake (happens with old python versions)
if(PYTHON_EXECUTABLE)
    execute_process(COMMAND "${PYTHON_EXECUTABLE}" --version
                    ERROR_VARIABLE _VERSION
                    RESULT_VARIABLE _PYTHON_VERSION_RESULT
                    OUTPUT_QUIET
                    ERROR_STRIP_TRAILING_WHITESPACE)
    if(_PYTHON_VERSION_RESULT)
        execute_process(COMMAND "${PYTHON_EXECUTABLE}" -V
                        ERROR_VARIABLE _VERSION
                        RESULT_VARIABLE _PYTHON_VERSION_RESULT
                        OUTPUT_QUIET
                        ERROR_STRIP_TRAILING_WHITESPACE)
    endif(_PYTHON_VERSION_RESULT)
    if(NOT _PYTHON_VERSION_RESULT AND _VERSION MATCHES "^Python [0-9]+\\.[0-9]+.*")
        string(REPLACE "Python " "" PYTHON_VERSION_STRING "${_VERSION}")
        string(REGEX REPLACE "^([0-9]+)\\.[0-9]+.*" "\\1" PYTHON_VERSION_MAJOR "${PYTHON_VERSION_STRING}")
        string(REGEX REPLACE "^[0-9]+\\.([0-9])+.*" "\\1" PYTHON_VERSION_MINOR "${PYTHON_VERSION_STRING}")
        if(PYTHON_VERSION_STRING MATCHES "^[0-9]+\\.[0-9]+\\.[0-9]+.*")
            string(REGEX REPLACE "^[0-9]+\\.[0-9]+\\.([0-9]+).*" "\\1" PYTHON_VERSION_PATCH "${PYTHON_VERSION_STRING}")
        endif()
    endif()
endif(PYTHON_EXECUTABLE)

if(PYTHONINTERP_FOUND AND NOT PYTHON_VERSION_STRING VERSION_LESS "2.5" )
	#run python to create the wrapper
		add_custom_command(
			OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/tigl3/tigl3wrapper.py
			DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/make_tigl_python.py ${PROJECT_SOURCE_DIR}/src/api/tigl.h
			WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
			COMMAND ${PYTHON_EXECUTABLE}
			ARGS ${CMAKE_CURRENT_SOURCE_DIR}/make_tigl_python.py
		)

		add_custom_target(tiglPython ALL
			COMMENT "Create python interface for tigl" VERBATIM 
			DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/tigl3/tigl3wrapper.py
		)
		
		install(FILES ${CMAKE_CURRENT_BINARY_DIR}/tigl3/tigl3wrapper.py
				DESTINATION share/tigl3/python/tigl3
				COMPONENT interfaces)
else()
    message(ERROR "Python could not be found or the python installation is too old ( < 2.5). Python bindings can not be build!")
endif()

endif (TIGL_BINDINGS_PYTHON)


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

    find_package(Python3 COMPONENTS Interpreter Development REQUIRED)
    message(STATUS "Python3 interpreter:" ${Python3_EXECUTABLE})
    message(STATUS "Python include directory: ${Python3_INCLUDE_DIR}")
    message(STATUS "Python library release: ${Python3_LIBRARY_RELEASE}")


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

    # we use the same definitions as pythonocc for their build to hide some of their warnings
    option(SWIG_HIDE_WARNINGS "Check this option if you want a less verbose swig output." ON)
    set(CMAKE_SWIG_FLAGS ${CMAKE_SWIG_FLAGS} -py3)
    if(SWIG_HIDE_WARNINGS)
      message(STATUS "Disabled SWIG warnings")
      set(CMAKE_SWIG_FLAGS ${CMAKE_SWIG_FLAGS} -w302,401,402,412,314,509,512,504,325,503,520,350,351,383,389,394,395,404)
    endif()


    include_directories(${PYTHON_INCLUDE_PATH})

    include_directories(${PythonOCC_INCLUDE_DIRS})

    set (MODULES "geometry;occtaddons;tmath;configuration;core;boolean_ops;exports;imports")

    set(LIBRARY_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR}/tigl3)
    #set(CMAKE_SWIG_OUTDIR ${CMAKE_CURRENT_BINARY_DIR}/tigl3)

    add_definitions(-DTIGL_INTERNAL_IMPORTS)

    if (PYTHONNOCC_LEGACY)
        set(CMAKE_SWIG_FLAGS ${CMAKE_SWIG_FLAGS} -DPYTHONNOCC_LEGACY)
        add_definitions(-DPYTHONNOCC_LEGACY)
    else(PYTHONNOCC_LEGACY)
        # This directory will only we rea at least and includes file of pythonocc < 7
        include_directories(${CMAKE_CURRENT_SOURCE_DIR}/compat )
    endif(PYTHONNOCC_LEGACY)

    include_directories($<TARGET_PROPERTY:tigl3_cpp,INTERFACE_INCLUDE_DIRECTORIES>)
    include_directories($<TARGET_PROPERTY:tigl3,INTERFACE_INCLUDE_DIRECTORIES>)

    foreach(MODULE ${MODULES})
        set_source_files_properties(${MODULE}.i PROPERTIES CPLUSPLUS ON)
        set(SWIG_MODULE_${MODULE}_EXTRA_DEPS common.i doc.i)

        swig_add_library(${MODULE} LANGUAGE python SOURCES ${MODULE}.i TYPE MODULE OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/tigl3)
        swig_link_libraries(${MODULE} tigl3 tigl3_cpp Python3::Module)

        install(TARGETS _${MODULE}
                DESTINATION share/tigl3/python/tigl3
                COMPONENT interfaces
        )

        install(FILES ${CMAKE_CURRENT_BINARY_DIR}/tigl3/${MODULE}.py
                DESTINATION share/tigl3/python/tigl3
                COMPONENT interfaces
        )

    endforeach(MODULE)

    file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/tigl3 DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

    install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tigl3
            DESTINATION share/tigl3/python
            COMPONENT interfaces
    )


endif(TIGL_BINDINGS_PYTHON_INTERNAL)
