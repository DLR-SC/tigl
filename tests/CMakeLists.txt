# units tests makefile

include(tiglmacros)

FILE (GLOB_RECURSE test_SRCS *.cpp *.cxx *.cc *.C *.c *.h *.hpp)
SET (test_LIBS gtest TIGL_static)
SET (test_BIN ${PROJECT_NAME}-unittests)

# add all tigl directories to include path
set(TIGL_INCLUDES ${PROJECT_SOURCE_DIR}/src/)
subdirlist(SUBDIRS ${PROJECT_SOURCE_DIR}/src/)
foreach(subdir ${SUBDIRS})
  set(TIGL_INCLUDES ${TIGL_INCLUDES} ${PROJECT_SOURCE_DIR}/src/${subdir})
endforeach()
INCLUDE_DIRECTORIES(${TIGL_INCLUDES})

INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR} ${PROJECT_SOURCE_DIR}/src ${PROJECT_BINARY_DIR}/src ${PROJECT_BINARY_DIR}/src/api ${XML_INCLUDE_DIRS} ${TIXI_INCLUDE_DIRS} ${OpenCASCADE_INCLUDE_DIR})
# boost
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/thirdparty/boost_1_55_0)

LINK_DIRECTORIES(${LIBRARY_OUTPUT_PATH})
ADD_EXECUTABLE(${test_BIN} ${test_SRCS} )

TARGET_LINK_LIBRARIES(${test_BIN} ${test_LIBS})

if(GLOG_FOUND)
  include_directories(${GLOG_INCLUDE_DIR})
  TARGET_LINK_LIBRARIES(${test_BIN} ${GLOG_LIBRARIES})
endif(GLOG_FOUND)

# boost
include_directories(${PROJECT_SOURCE_DIR}/thirdparty/boost_1_55_0)

ADD_CUSTOM_TARGET(check ${test_BIN} --gtest_output=xml DEPENDS ${test_BIN} COMMENT "Executing unit tests..." VERBATIM SOURCES ${test_SRCS})

ADD_TEST(
  NAME unittests
  COMMAND ${test_BIN} --gtest_output=xml
)

#execute_process(COMMAND ${CMAKE_PROGRAM} -E copy_if_different ${PROJECT_SOURCE_DIR}/tests/TestData tests/Testdata)
file(COPY ${PROJECT_SOURCE_DIR}/tests/TestData DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/pythonwrapper-tests.py DESTINATION ${PROJECT_BINARY_DIR}/tests)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/test_logging.py DESTINATION ${PROJECT_BINARY_DIR}/tests)

#create directory to avoid crash in unit tests
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/TestData/export)
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/TestData/analysis)

configure_file (
  "${CMAKE_CURRENT_SOURCE_DIR}/runtests.sh.in"
  "${CMAKE_CURRENT_BINARY_DIR}/runtests.sh"
)

AddToCheckstyle()

