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

source_group(" " FILES ${ALL_SRCS} ${ALL_HEADERS})

# TODO this is VS specific
add_custom_command(
	TARGET ${PROJECT_NAME}
	POST_BUILD
	COMMAND ${CMAKE_CURRENT_BINARY_DIR}/$(Configuration)/${PROJECT_NAME}.exe
	COMMENT Running CPACSGen
	VERBATIM
)

set(GENERATED_CPACS GeneratedCPACS)
file(GLOB ALL_GENERATED_SRCS ${CMAKE_CURRENT_BINARY_DIR}/generated/*.cpp)
file(GLOB ALL_GENERATED_HEADERS ${CMAKE_CURRENT_BINARY_DIR}/generated/*.h*)
if (MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /bigobj /MP")
endif()
add_executable(${GENERATED_CPACS} ${ALL_GENERATED_SRCS} ${ALL_GENERATED_HEADERS})
add_dependencies(${GENERATED_CPACS} ${PROJECT_NAME})

# from: http://stackoverflow.com/questions/7787823/cmake-how-to-get-the-name-of-all-subdirectories-of-a-directory
MACRO(SUBDIRLIST result curdir)
  FILE(GLOB children RELATIVE ${curdir} ${curdir}/*)
  SET(dirlist "")
  FOREACH(child ${children})
    IF(IS_DIRECTORY ${curdir}/${child})
        LIST(APPEND dirlist ${child})
    ENDIF()
  ENDFOREACH()
  SET(${result} ${dirlist})
ENDMACRO()

set(TIGL_INCLUDES ${CMAKE_CURRENT_SOURCE_DIR}/../src)
SUBDIRLIST(SUBDIRS ${CMAKE_CURRENT_SOURCE_DIR}/../src)
foreach(subdir ${SUBDIRS})
  set(TIGL_INCLUDES ${TIGL_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR}/../src/${subdir})
endforeach()
include_directories(${GENERATED_CPACS} ${TIGL_INCLUDES})