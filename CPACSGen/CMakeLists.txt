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
file(GLOB ALL_TABLES src/*Table.*)

#set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)

include_directories(${TIXI_INCLUDE_DIRS})
add_executable(${PROJECT_NAME} ${ALL_SRCS} ${ALL_HEADERS})
target_link_libraries(${PROJECT_NAME} ${TIXI_LIBRARIES})

source_group(" " FILES ${ALL_SRCS} ${ALL_HEADERS})
source_group("Tables" FILES ${ALL_TABLES})

# generate files side by side to TIGL source
set(GENERATOR_OUTPUT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../src/generated)

# TODO generate post build action to copy IOHelper and related classes

# TODO this is VS specific
add_custom_command(
	TARGET ${PROJECT_NAME}
	POST_BUILD
	COMMAND ${CMAKE_CURRENT_BINARY_DIR}/$(Configuration)/${PROJECT_NAME}.exe
	ARGS ${GENERATOR_OUTPUT_DIR}
	COMMENT Running CPACSGen
	VERBATIM
)
