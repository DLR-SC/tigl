# tigl demo cmake project
# author: Martin Siggel

include_directories(${PROJECT_SOURCE_DIR}/src ${TIXI_INCLUDE_DIR})

add_executable(tiglDemo	tiglDemo.c)
target_link_libraries(tiglDemo TIGL)

