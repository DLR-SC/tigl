# TIGL Library

source_group(Fuselage REGULAR_EXPRESSION "CCPACSFuselage")
source_group(Wing REGULAR_EXPRESSION "CCPACSWing")

include_directories(${OpenCASCADE_INCLUDE_DIR} ${TIXI_INCLUDE_DIRS})

add_definitions(-DTIGL_VERSION=\"${TIGL_VERSION}\")

file(GLOB TIGL_SRC
    "*.h"
    "*.cpp"
)

add_library(TIGL SHARED ${TIGL_SRC})
target_link_libraries (TIGL ${OpenCASCADE_LIBRARIES} ${TIXI_LIBRARIES})

add_library(TIGL_static ${TIGL_SRC})
