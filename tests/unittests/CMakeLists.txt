file (GLOB test_SRCS *.cpp *.cxx *.cc *.C *.c *.h *.hpp)

set (test_BIN "TiGL-unittests")


add_executable(${test_BIN} ${test_SRCS} )
target_link_libraries(${test_BIN} PUBLIC gtest tigl3_static tigl3_cpp tiglCommonTestUtils)

ADD_CUSTOM_TARGET(check_tigl ${test_BIN} --gtest_output=xml DEPENDS ${test_BIN} COMMENT "Executing unit tests..." VERBATIM SOURCES ${test_SRCS})

ADD_TEST(
  NAME unittests
  COMMAND ${test_BIN} --gtest_output=xml
)

#execute_process(COMMAND ${CMAKE_PROGRAM} -E copy_if_different ${PROJECT_SOURCE_DIR}/tests/TestData tests/Testdata)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/TestData DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/pythonwrapper-tests.py DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/test_logging.py DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

#create directory to avoid crash in unit tests
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/TestData/export)
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/TestData/analysis)
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/TestData/bugs/505)

configure_file (
  "${CMAKE_CURRENT_SOURCE_DIR}/runtests.sh.in"
  "${CMAKE_CURRENT_BINARY_DIR}/runtests.sh"
)
