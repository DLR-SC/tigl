find_package(PythonInterp)

if(PYTHONINTERP_FOUND)
	
	configure_file(${CMAKE_CURRENT_SOURCE_DIR}/make-wrapper.py.in ${CMAKE_CURRENT_BINARY_DIR}/make-wrapper.py)
	
	#run python to create the wrapper
		add_custom_command(
			OUTPUT ${LIBRARY_OUTPUT_PATH}/tiglwrapper.py
			DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/make-wrapper.py
			WORKING_DIRECTORY ${LIBRARY_OUTPUT_PATH}
			COMMAND ${PYTHON_EXECUTABLE}
			ARGS ${CMAKE_CURRENT_BINARY_DIR}/make-wrapper.py
		)

		add_custom_target(tiglPython ALL
			COMMENT "Create python interface for tigl" VERBATIM 
			DEPENDS ${LIBRARY_OUTPUT_PATH}/tiglwrapper.py
		)
		
		install(FILES ${LIBRARY_OUTPUT_PATH}/tiglwrapper.py 
				DESTINATION share/tigl/python
				COMPONENT interfaces)
endif()