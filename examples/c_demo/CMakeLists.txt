cmake_minimum_required(VERSION 3.14)

# tigl demo cmake project
# author: Martin Siggel
project(tigl-demo)

if (NOT TARGET tigl3)
    find_package(tigl3 CONFIG REQUIRED)
endif()

add_executable(c_demo c_demo.c)
target_link_libraries(c_demo tigl3 ${OpenCASCADE_LIBRARIES})

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
