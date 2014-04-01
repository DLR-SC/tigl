# tigl demo cmake project
# author: Martin Siggel

include_directories(${PROJECT_SOURCE_DIR}/src/api ${TIXI_INCLUDE_DIRS})


set(demo_LIBS TIGL)

add_executable(tiglDemo	tiglDemo.c)
target_link_libraries(tiglDemo ${demo_LIBS})

