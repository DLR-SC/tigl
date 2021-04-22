file (GLOB test_SRCS *.cpp *.cxx *.cc *.C *.c *.h *.hpp)

set (test_BIN "TiGL-integrationtests")


add_executable(${test_BIN} ${test_SRCS} )
target_link_libraries(${test_BIN} PUBLIC gtest tigl3_static tigl3_cpp tiglCommonTestUtils)

ADD_TEST(
  NAME integrationtests
  COMMAND ${test_BIN} --gtest_output=xml
)

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/TestData DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

#create directory to avoid crash in unit tests
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/TestData/export)
