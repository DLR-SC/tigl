# tigl demo cmake project
# author: Martin Siggel

include_directories(${PROJECT_SOURCE_DIR}/src/api ${TIXI_INCLUDE_DIRS})


set(demo_LIBS tigl3)

add_executable(c_demo c_demo.c)
target_link_libraries(c_demo ${demo_LIBS})

install(FILES 
    c_demo.c
    DESTINATION share/doc/tigl3/examples
    COMPONENT docu
)

if(WIN32)
  # install makefile.msvc
  install(FILES 
      makefile.msvc
      DESTINATION share/doc/tigl3/examples
      COMPONENT docu
  )
else(WIN32)
  # install makefile.gnu  
  install(FILES 
      makefile.gnu
      DESTINATION share/doc/tigl3/examples
      COMPONENT docu
  )
endif(WIN32)
