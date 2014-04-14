# tigl demo cmake project
# author: Martin Siggel

include_directories(${PROJECT_SOURCE_DIR}/src/api ${TIXI_INCLUDE_DIRS})


set(demo_LIBS TIGL)

add_executable(c_demo c-demo.c)
target_link_libraries(c_demo ${demo_LIBS})

install(FILES c-demo.c
    DESTINATION share/doc/tigl/examples
    COMPONENT docu
)
