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

	set_source_files_properties(geometry.i PROPERTIES CPLUSPLUS ON)
        set_source_files_properties(occtaddons.i PROPERTIES CPLUSPLUS ON)
	set_source_files_properties(tmath.i PROPERTIES CPLUSPLUS ON)

	swig_add_module(occtaddons python occtaddons.i )
	swig_link_libraries(occtaddons ${PYTHON_LIBRARIES} TIGL)

	swig_add_module(geometry python geometry.i )
	swig_link_libraries(geometry ${PYTHON_LIBRARIES} TIGL)
	
	swig_add_module(tmath python tmath.i )
	swig_link_libraries(tmath ${PYTHON_LIBRARIES} TIGL)
endif(TIGL_PYTHON_INTERNAL)
