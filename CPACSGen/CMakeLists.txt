# CPACSGen cmake project
# author: Bernhard Manfred Gruber

#include(tiglmacros)
#AddToCheckstyle()

# boost
#include_directories(${PROJECT_SOURCE_DIR}/thirdparty/boost_1_55_0)

file(GLOB ALL_SRCS src/*.cpp)
file(GLOB ALL_HEADERS src/*.h)

#set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)

add_executable("CPACSGen" ${ALL_SRCS} ${ALL_HEADERS})
