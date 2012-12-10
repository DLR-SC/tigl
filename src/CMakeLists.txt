# TIGL Library

source_group(Fuselage REGULAR_EXPRESSION "CCPACSFuselage")
source_group(Wing REGULAR_EXPRESSION "CCPACSWing")

include_directories(${OpenCASCADE_INCLUDE_DIR} ${TIXI_INCLUDE_DIRS})

add_definitions(-DTIGL_VERSION=\"${TIGL_VERSION}\")

file(GLOB TIGL_SRC
    "*.h"
    "*.cpp"
)

if(MSVC)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS=1)
endif(MSVC)

# tigl dll/so library
add_library(TIGL SHARED ${TIGL_SRC})
target_link_libraries (TIGL ${OpenCASCADE_LIBRARIES} ${TIXI_LIBRARIES} ${VLD_LIBRARIES})
set_target_properties( TIGL PROPERTIES VERSION ${TIGL_VERSION})
set_target_properties( TIGL PROPERTIES SOVERSION "${TIGL_VERSION_MAJOR}")

install (TARGETS TIGL 
		 EXPORT tigl-targets
		 RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
		 LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
		 ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
		 )
		 
install (EXPORT tigl-targets DESTINATION ${CMAKE_INSTALL_LIBDIR}/tigl)			 
		 
install (FILES tigl.h 
		 DESTINATION include
		 COMPONENT headers)		 

# static lib for tiglviewer		 
add_library(TIGL_static ${TIGL_SRC})




		 

		 
