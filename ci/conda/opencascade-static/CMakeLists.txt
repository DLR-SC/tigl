# a little test to see if occt is installed
project(occt_test_cmake NONE)
cmake_minimum_required(VERSION 3.5)

find_package(OpenCASCADE)
if (OpenCASCADE_MAJOR_VERSION)
  message(STATUS " Found OpenCASCADE version: ${OpenCASCADE_MAJOR_VERSION}.${OpenCASCADE_MINOR_VERSION}.${OpenCASCADE_MAINTENANCE_VERSION}")
else(OpenCASCADE_MAJOR_VERSION)
  message(FATAL_ERROR "could not find OpenCASCADE")
endif(OpenCASCADE_MAJOR_VERSION)