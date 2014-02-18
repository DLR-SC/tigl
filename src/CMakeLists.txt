# TIGL Library

find_package(Threads REQUIRED)

configure_file (
  "${CMAKE_CURRENT_SOURCE_DIR}/tigl_version.h.in"
  "${CMAKE_CURRENT_BINARY_DIR}/tigl_version.h"
)

source_group(Fuselage REGULAR_EXPRESSION "CCPACSFuselage")
source_group(Wing REGULAR_EXPRESSION "CCPACSWing")

include_directories(${OpenCASCADE_INCLUDE_DIR}
                    ${TIXI_INCLUDE_DIRS}
                    ${CMAKE_CURRENT_BINARY_DIR}
                    ${CMAKE_CURRENT_SOURCE_DIR}
                    ${CMAKE_CURRENT_SOURCE_DIR}/boolean_operations
)

file(GLOB_RECURSE TIGL_SRC
    "*.h"
    "*.hxx"
    "*.cpp"
    "*.cxx"
)

if(MSVC)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS=1 /wd4355)
endif(MSVC)

# tigl dll/so library
add_library(TIGL SHARED ${TIGL_SRC})

if(GLOG_FOUND)
  include_directories(${GLOG_INCLUDE_DIR})
  target_link_libraries (TIGL ${GLOG_LIBRARIES})
endif(GLOG_FOUND)

# boost
include_directories(${PROJECT_SOURCE_DIR}/thirdparty/boost_1_55_0)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  add_definitions(-DDEBUG)
endif()

set(TIGL_LIBRARIES ${OpenCASCADE_LIBRARIES} ${TIXI_LIBRARIES} ${VLD_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  set(TIGL_LIBRARIES  ${TIGL_LIBRARIES} rt)
endif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")

target_link_libraries (TIGL ${TIGL_LIBRARIES})
set_target_properties( TIGL PROPERTIES VERSION ${TIGL_VERSION})
set_target_properties( TIGL PROPERTIES SOVERSION "${TIGL_VERSION_MAJOR}")

install (TARGETS TIGL 
		 EXPORT tigl-targets
		 RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
		 LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
		 ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
		 )
		 
install (EXPORT tigl-targets DESTINATION ${CMAKE_INSTALL_LIBDIR}/tigl)
		 
install (FILES tigl.h ${CMAKE_CURRENT_BINARY_DIR}/tigl_version.h
		 DESTINATION include
		 COMPONENT headers)

# static lib for tiglviewer 
add_library(TIGL_static ${TIGL_SRC})
# this does no actual linking but keeps track of the dependencies
target_link_libraries (TIGL_static ${TIGL_LIBRARIES})
