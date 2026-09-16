# bindings/CMakeLists.txt already runs find_package(Python3 ... REQUIRED) before
# adding this subdirectory, so PYTHON_EXECUTABLE/Python3_VERSION are always valid
# here. (The previous hand-rolled `python --version` parsing below this comment
# used to work around ancient CMake's lack of FindPython3, but had been silently
# broken since Python 3.4, which prints `--version` to stdout, not stderr.)
if(NOT Python3_VERSION VERSION_LESS "2.5")
	#run python to create the wrapper
		add_custom_command(
			OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/tigl3wrapper.py
			DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/make_tigl_python.py ${PROJECT_SOURCE_DIR}/src/api/tigl.h
			WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
			COMMAND ${PYTHON_EXECUTABLE}
			ARGS ${CMAKE_CURRENT_SOURCE_DIR}/make_tigl_python.py
		)

		add_custom_target(tiglPython ALL
			COMMENT "Create python interface for tigl" VERBATIM
			DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/tigl3wrapper.py
		)

		install(FILES ${CMAKE_CURRENT_BINARY_DIR}/tigl3wrapper.py
				DESTINATION ${PYTHON_SITE_PACKAGES}/tigl3
				COMPONENT interfaces)
else()
    message(WARNING "Python installation is too old ( < 2.5). Python bindings can not be built!")
endif()
