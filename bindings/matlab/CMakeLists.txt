# Matlab bindings for tigl
# @author Martin Siggel

option(TIGL_BINDINGS_MATLAB "Build the matlab bindings of tigl (requires matlab and python)" OFF)


if(TIGL_BINDINGS_MATLAB)

find_package(MATLAB REQUIRED)
find_package(PythonInterp REQUIRED)

message(STATUS "Found MATLAB. Create matlab bindings.")

#run python to create the tiglmatlab.c
add_custom_command(
	OUTPUT tiglmatlab.c
	DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/make_tigl_matlab.py ${PROJECT_SOURCE_DIR}/src/api/tigl.h
	WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
	COMMAND ${PYTHON_EXECUTABLE}
	ARGS ${CMAKE_CURRENT_SOURCE_DIR}/make_tigl_matlab.py
)

# c wrapper code
set(TMAT_SRC tiglmatlab.c common.h)

# add def for exports
if(MSVC)
  set(TMAT_SRC ${TMAT_SRC} exports.def)
endif()

include_directories(${PROJECT_SOURCE_DIR}/src/api ${MATLAB_INCLUDE_DIR} ${TIXI_INCLUDE_DIRS} ${CMAKE_CURRENT_SOURCE_DIR})

add_library(tigl_matlab SHARED ${TMAT_SRC})
target_link_libraries(tigl_matlab ${MATLAB_mat_LIBRARY} ${MATLAB_mx_LIBRARY} ${MATLAB_mex_LIBRARY} TIGL)
set_target_properties(tigl_matlab PROPERTIES SUFFIX ${MATLAB_MEX_SUFFIX} PREFIX "")

install(TARGETS tigl_matlab DESTINATION share/tigl/matlab
	COMPONENT interfaces)
		
install(
    CODE "file( GLOB MFILES ${CMAKE_CURRENT_BINARY_DIR}/*.m )"
    CODE "file( INSTALL \${MFILES} DESTINATION \${CMAKE_INSTALL_PREFIX}/share/tigl/matlab )"
	COMPONENT interfaces
)

endif() # matlab bindings enabled
