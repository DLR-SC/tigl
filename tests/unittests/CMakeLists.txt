# Set a default build type if none was specified
if(NOT DEFINED CMAKE_BUILD_TYPE)
  message(STATUS "Setting build type to 'Release' as none was specified.")
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build." FORCE)
  # Set the possible values of build type for cmake-gui
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release"
    "MinSizeRel" "RelWithDebInfo")
endif()

cmake_minimum_required(VERSION 3.0)
project (TiGL-unittests)

list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

if (NOT TARGET tigl3_cpp)
    find_package(tigl3_cpp CONFIG REQUIRED)
endif()

if (NOT TARGET gtest)
    include(AddGoogleTest)
endif()

file (GLOB test_SRCS *.cpp *.cxx *.cc *.C *.c *.h *.hpp)

set (test_BIN ${PROJECT_NAME})


add_executable(${test_BIN} ${test_SRCS} )
target_link_libraries(${test_BIN} PUBLIC gtest tigl3_static tigl3_cpp)

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
