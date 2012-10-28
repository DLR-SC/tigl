# units tests makefile

FILE (GLOB_RECURSE test_SRCS *.cpp *.cxx *.cc *.C *.c *.h *.hpp)
SET (test_LIBS gtest TIGL)
SET (test_BIN ${PROJECT_NAME}-unittests)

INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR} ${PROJECT_SOURCE_DIR}/src ${XML_INCLUDE_DIRS}, ${TIXI_INCLUDE_DIRS})

LINK_DIRECTORIES(${LIBRARY_OUTPUT_PATH})
ADD_EXECUTABLE(${test_BIN} ${test_SRCS} )
TARGET_LINK_LIBRARIES(${test_BIN} ${test_LIBS})
ADD_CUSTOM_TARGET(check ${test_BIN} --gtest_output=xml DEPENDS ${test_BIN} COMMENT "Executing unit tests..." VERBATIM SOURCES ${test_SRCS})
ADD_CUSTOM_TARGET(test DEPENDS check)

#execute_process(COMMAND ${CMAKE_PROGRAM} -E copy_if_different ${PROJECT_SOURCE_DIR}/tests/TestData tests/Testdata)
file(COPY ${PROJECT_SOURCE_DIR}/tests/TestData DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
