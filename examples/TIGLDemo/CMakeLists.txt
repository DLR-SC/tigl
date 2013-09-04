# tigl demo cmake project
# author: Martin Siggel

include_directories(${PROJECT_SOURCE_DIR}/src ${TIXI_INCLUDE_DIR})


set(demo_LIBS TIGL)

add_executable(tiglDemo	tiglDemo.c)
target_link_libraries(tiglDemo ${demo_LIBS})

