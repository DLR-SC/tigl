# Set a default build type if none was specified
if(NOT DEFINED CMAKE_BUILD_TYPE)
  message(STATUS "Setting build type to 'Release' as none was specified.")
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build." FORCE)
  # Set the possible values of build type for cmake-gui
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release"
    "MinSizeRel" "RelWithDebInfo")
endif()

cmake_minimum_required(VERSION 3.0)
project (TiGL-tests)

list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

if (NOT TARGET tigl3_cpp)
    find_package(tigl3_cpp CONFIG REQUIRED)
endif()

include(AddGoogleTest)

add_subdirectory(common)
add_subdirectory(unittests)
add_subdirectory(integrationtests)
