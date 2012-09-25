# TIGL units tests makefile

find_package(CUNIT)

if(CUNIT_FOUND)

set(TEST_FUNCTS_SRC 
	cpacsConfiguration.c
	testFunctions.h
	tiglAlgorithm.c	
	tiglExports.c
	tiglFuselageGetPoint.c
	tiglFuselageSegment.c
	tiglIntersectionCalculation.c
	tiglWing.c
	tiglWingGetPoint.c
	tiglWingSegment.c
)

set(TEST_SUITE_SRC CUMain.c addTestFunctions.c)

foreach(FILE ${TEST_FUNCTS_SRC})
	set(TEST_SUITE_SRC ${TEST_SUITE_SRC} TestFunctions/${FILE})
endforeach()

include_directories(${CUNIT_INCLUDE_DIRS} ${XML_INCLUDE_DIRS} ${TIXI_INCLUDE_DIRS})
include_directories(${PROJECT_SOURCE_DIR}/src ${CMAKE_CURRENT_SOURCE_DIR}/TestFunctions)

add_executable(CUnitTests ${TEST_SUITE_SRC})
target_link_libraries(CUnitTests ${CUNIT_LIBRARIES} TIGL)

# copy the test data to the build directory
add_custom_command(
	TARGET CUnitTests
	POST_BUILD
	COMMAND ${CMAKE_COMMAND}
	ARGS -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/TestData/ ${CMAKE_CURRENT_BINARY_DIR}/TestData
)

endif(CUNIT_FOUND)

