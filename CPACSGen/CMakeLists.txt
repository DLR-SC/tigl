# CPACSGen cmake project
# author: Bernhard Manfred Gruber

cmake_minimum_required(VERSION 2.6)

project(CPACSGen)

#include(tiglmacros)
#AddToCheckstyle()

# boost
#include_directories(${PROJECT_SOURCE_DIR}/thirdparty/boost_1_55_0)
set(Boost_USE_STATIC_LIBS ON)
set(BOOST_ROOT C:/dev/descartes_sdk_1.10_win64/boost)
find_package (Boost REQUIRED)

# tixi
find_package(TIXI 2.1.0 REQUIRED)
if(TIXI_VERSION VERSION_GREATER 2.1.0)
  set(HAVE_TIXI_SETPRINTMSG TRUE)
endif()

file(GLOB ALL_SRCS src/*.cpp)
file(GLOB ALL_HEADERS src/*.h*)

#set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)

include_directories(${TIXI_INCLUDE_DIRS})

add_executable(${PROJECT_NAME} ${ALL_SRCS} ${ALL_HEADERS})

target_link_libraries(${PROJECT_NAME} ${TIXI_LIBRARIES})
