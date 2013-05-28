# units tests makefile

FILE (GLOB_RECURSE test_SRCS *.cpp *.cxx *.cc *.C *.c *.h *.hpp)
SET (test_LIBS gtest TIGL_static ${OpenCASCADE_LIBRARIES} ${TIXI_LIBRARIES} ${VLD_LIBRARIES})
SET (test_BIN ${PROJECT_NAME}-unittests)

INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR} ${PROJECT_SOURCE_DIR}/src ${PROJECT_BINARY_DIR}/src ${XML_INCLUDE_DIRS} ${TIXI_INCLUDE_DIRS} ${OpenCASCADE_INCLUDE_DIR})

ADD_DEFINITIONS(-DTIGL_VERSION=\"${TIGL_VERSION}\")

LINK_DIRECTORIES(${LIBRARY_OUTPUT_PATH})
ADD_EXECUTABLE(${test_BIN} ${test_SRCS} )

TARGET_LINK_LIBRARIES(${test_BIN} ${test_LIBS})

if(GLOG_FOUND)
  message(STATUS "HELLO")
  include_directories(${GLOG_INCLUDE_DIR})
  TARGET_LINK_LIBRARIES(${test_BIN} ${GLOG_LIBRARIES})
endif(GLOG_FOUND)

ADD_CUSTOM_TARGET(check ${test_BIN} --gtest_output=xml DEPENDS ${test_BIN} COMMENT "Executing unit tests..." VERBATIM SOURCES ${test_SRCS})
ADD_CUSTOM_TARGET(test DEPENDS check)

#execute_process(COMMAND ${CMAKE_PROGRAM} -E copy_if_different ${PROJECT_SOURCE_DIR}/tests/TestData tests/Testdata)
file(COPY ${PROJECT_SOURCE_DIR}/tests/TestData DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/pythonwrapper-tests.py DESTINATION ${PROJECT_BINARY_DIR}/tests)

#create directory to avoid crash in unit tests
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/TestData/export)
