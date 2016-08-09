# CPACSGen cmake project
# author: Bernhard Manfred Gruber

cmake_minimum_required(VERSION 2.6)

project(CPACSGen)

# find boost
set(Boost_USE_STATIC_LIBS ON)
set(BOOST_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../thirdparty/boost_1_55_0)
find_package (Boost 1.55.0 REQUIRED)

# find tixi
set(TIXI_PATH "" CACHE PATH "TiXI installation prefix")
set(CMAKE_PREFIX_PATH "${TIXI_PATH};${CMAKE_PREFIX_PATH}")
find_package(TIXI 2.1.0 REQUIRED)
if(TIXI_VERSION VERSION_GREATER 2.1.0)
  set(HAVE_TIXI_SETPRINTMSG TRUE)
endif()

# source files
file(GLOB ALL_SRCS src/*.cpp)
file(GLOB ALL_HEADERS src/*.h*)
file(GLOB ALL_TABLES src/*Table*)

#includes 
include_directories(${Boost_INCLUDE_DIRS})
include_directories(${TIXI_INCLUDE_DIRS})

# exe
add_executable(${PROJECT_NAME} ${ALL_SRCS} ${ALL_HEADERS})

# library dependencies
target_link_libraries(${PROJECT_NAME} ${TIXI_LIBRARIES})

source_group(" " FILES ${ALL_SRCS} ${ALL_HEADERS})
source_group("Tables" FILES ${ALL_TABLES})

# add post build step to generate files side by side to TIGL source
set(GENERATOR_OUTPUT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../src/generated CACHE PATH "The output directory for the generated CPACS classes")
set(CPACS_SCHEMA_DIR ${CMAKE_CURRENT_SOURCE_DIR}/schema/cpacs_schema.xsd CACHE FILEPATH "The location of the XML schema file specifying CPACS")

# determine location if compiled generator executable (TODO: not sure if this is correct for all platforms)
if(MSVC)
	set(CPACSGEN_BINARY ${CMAKE_CURRENT_BINARY_DIR}/$(Configuration)/${PROJECT_NAME}.exe)
else()
	set(CPACSGEN_BINARY ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME})
endif()

add_custom_command(
	TARGET ${PROJECT_NAME}
	POST_BUILD
	COMMAND ${CPACSGEN_BINARY}
	ARGS ${CPACS_SCHEMA_DIR} ${GENERATOR_OUTPUT_DIR}
	COMMENT Running CPACSGen
	VERBATIM
)

if (NOT WIN32)
	set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif (NOT WIN32)


# copy runtime files used by generated code
set(RUNTIME_FILES
	${CMAKE_CURRENT_SOURCE_DIR}/src/TixiHelper.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/TixiHelper.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/Optional.hpp
)
file(COPY ${RUNTIME_FILES} DESTINATION ${GENERATOR_OUTPUT_DIR})
