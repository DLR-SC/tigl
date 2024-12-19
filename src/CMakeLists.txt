# TIGL Library

find_package(Threads REQUIRED)
include(tiglmacros)

if (NOT TARGET Boost::boost)
    find_package(Boost REQUIRED COMPONENTS filesystem system thread atomic chrono date_time)
endif()

configure_file (
  "${CMAKE_CURRENT_SOURCE_DIR}/api/tigl_version.h.in"
  "${CMAKE_CURRENT_BINARY_DIR}/api/tigl_version.h"
)

configure_file (
    "${CMAKE_CURRENT_SOURCE_DIR}/tigl_config.h.in"
    "${CMAKE_CURRENT_BINARY_DIR}/tigl_config.h"
)

# add all subdirectories to include path
set(TIGL_INCLUDES ".")
subdirlist(SUBDIRS ${CMAKE_CURRENT_SOURCE_DIR})
foreach(subdir ${SUBDIRS})
  set(TIGL_INCLUDES ${TIGL_INCLUDES} ${subdir})
endforeach()

file(GLOB_RECURSE TIGL_SRC
    "*.h"
    "*.hxx"
    "*.cpp"
    "*.cxx"
)

option(TIGL_CONCAT_GENERATED_FILES "Concatenate all generated files into one. This speeds up compilation." ON)
if (TIGL_CONCAT_GENERATED_FILES)
    # Building the generated source files takes a lot of time
    # Since they are normally not changed, we concatenate all
    # files into the file src_generated.cpp

    file(GLOB GENERATED_SRC "generated/*.cpp")

    # remove all generated files from the tigl target
    list(REMOVE_ITEM TIGL_SRC ${GENERATED_SRC})

    configure_file (
      "${PROJECT_SOURCE_DIR}/cmake/concat_generated.cmake.in"
       "${PROJECT_BINARY_DIR}/cmake/concat_generated.cmake"
       @ONLY
    )

    add_custom_command(OUTPUT "${PROJECT_BINARY_DIR}/src/src_generated.cpp"
      DEPENDS ${GENERATED_SRC}
      COMMAND ${CMAKE_COMMAND} -P
      ${PROJECT_BINARY_DIR}/cmake/concat_generated.cmake
    )
    list(APPEND TIGL_SRC "${PROJECT_BINARY_DIR}/src/src_generated.cpp")
    if (MSVC)
      set_source_files_properties(${PROJECT_BINARY_DIR}/src/src_generated.cpp PROPERTIES COMPILE_FLAGS "/bigobj")
    endif(MSVC)

    if (MSVC)
         SET_SOURCE_FILES_PROPERTIES( "${PROJECT_BINARY_DIR}/src/src_generated.cpp" PROPERTIES COMPILE_FLAGS "/bigobj" )
    endif(MSVC)
    if (MINGW)
         SET_SOURCE_FILES_PROPERTIES( "${PROJECT_BINARY_DIR}/src/src_generated.cpp" PROPERTIES COMPILE_FLAGS "-Wa,-mbig-obj" )
    endif(MINGW)

else(TIGL_CONCAT_GENERATED_FILES)
    list(REMOVE_ITEM SUBDIRS generated)
endif(TIGL_CONCAT_GENERATED_FILES)

# group sources as on filesystem
foreach(source ${TIGL_SRC})
    get_filename_component(sourcePath "${source}" DIRECTORY)
    file(RELATIVE_PATH sourcePath ${CMAKE_CURRENT_SOURCE_DIR} ${sourcePath})
    string(REPLACE "/" "\\" sourcePathNoBS "${sourcePath}")
    source_group("${sourcePathNoBS}" FILES "${source}")
endforeach()

# object library containing just the compiled sources
add_library(tigl3_objects OBJECT ${TIGL_SRC})
set_property(TARGET tigl3_objects PROPERTY POSITION_INDEPENDENT_CODE ON) # needed for shared libraries

target_include_directories(tigl3_objects
    PRIVATE $<TARGET_PROPERTY:TKernel,INTERFACE_INCLUDE_DIRECTORIES>
    PRIVATE ${TIGL_INCLUDES}
    PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/api
    PRIVATE ${CMAKE_CURRENT_BINARY_DIR}
    PRIVATE $<TARGET_PROPERTY:Boost::boost,INTERFACE_INCLUDE_DIRECTORIES>
    PUBLIC $<TARGET_PROPERTY:tixi3,INTERFACE_INCLUDE_DIRECTORIES>
)

# This is a workaround for a tixi cmake bug:
#   A installed tixi does not yet have a target tixicpp
#   but the tixicpp includes are in the same dir as the tixi3 includes.
#   A local build has it but has a different include directory than tixi3.
if (TARGET tixicpp)
    target_include_directories(tigl3_objects
        PRIVATE $<TARGET_PROPERTY:tixicpp,INTERFACE_INCLUDE_DIRECTORIES>
    )
endif(TARGET tixicpp)

target_compile_definitions(tigl3_objects
    PRIVATE -Dtigl3_EXPORTS
    PRIVATE $<TARGET_PROPERTY:Boost::disable_autolinking,INTERFACE_COMPILE_DEFINITIONS>
    PRIVATE $<TARGET_PROPERTY:TKernel,INTERFACE_COMPILE_DEFINITIONS>
)

if(TARGET glog::glog)
  target_compile_definitions (tigl3_objects
    PRIVATE $<TARGET_PROPERTY:glog::glog,INTERFACE_COMPILE_DEFINITIONS>
  )

  target_include_directories(tigl3_objects
    PRIVATE $<TARGET_PROPERTY:glog::glog,INTERFACE_INCLUDE_DIRECTORIES>
  )
endif()

if(MSVC)
    target_compile_definitions(tigl3_objects PRIVATE _CRT_SECURE_NO_WARNINGS=1 /wd4355 TIGL_INTERNAL_EXPORTS)
endif(MSVC)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  target_compile_definitions(tigl3_objects PRIVATE DEBUG)
  if(DEBUG_EXTENSIVE)
      target_compile_definitions(tigl3_objects PRIVATE DEBUG_GUIDED_SURFACE_CREATION)
  endif()
endif()

# workaround for xcode. We must give each target a source file, not just a list of object files
file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/dummy.cpp "// DUMMY FILE")

# tigl dll/so library
add_library(tigl3 SHARED $<TARGET_OBJECTS:tigl3_objects> ${CMAKE_CURRENT_BINARY_DIR}/dummy.cpp)
target_compile_definitions(tigl3
    PRIVATE $<TARGET_PROPERTY:Boost::disable_autolinking,INTERFACE_COMPILE_DEFINITIONS>
)


target_link_libraries (tigl3 PRIVATE ${OpenCASCADE_LIBRARIES} ${VLD_LIBRARIES}
                             PRIVATE Boost::filesystem Boost::thread Boost::system Boost::atomic Boost::chrono Boost::date_time
                             PRIVATE ${CMAKE_THREAD_LIBS_INIT}
                             PUBLIC tixi3)

target_include_directories(tigl3 PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/api>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/api>
    $<INSTALL_INTERFACE:include/tigl3>
)

set_target_properties( tigl3 PROPERTIES VERSION ${TIGL_VERSION_MINOR}.${TIGL_VERSION_PATCH})
set_target_properties( tigl3 PROPERTIES SOVERSION "${TIGL_VERSION_MINOR}")

install (TARGETS tigl3
         EXPORT tigl3-targets
         RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
         LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
         ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

install (FILES api/tigl.h ${CMAKE_CURRENT_BINARY_DIR}/api/tigl_version.h
         DESTINATION include/tigl3
         COMPONENT headers)

# static lib for tiglviewer 
add_library(tigl3_static STATIC $<TARGET_OBJECTS:tigl3_objects> ${CMAKE_CURRENT_BINARY_DIR}/dummy.cpp)
# this does no actual linking but keeps track of the dependencies
target_link_libraries (tigl3_static
     PRIVATE ${VLD_LIBRARIES}
     PUBLIC Boost::disable_autolinking
     PUBLIC tixi3
     PRIVATE ${CMAKE_THREAD_LIBS_INIT}
     PRIVATE ${OpenCASCADE_LIBRARIES} Boost::boost Boost::filesystem Boost::thread Boost::system Boost::atomic Boost::chrono Boost::date_time
)

if (TARGET glog::glog)
  target_link_libraries (tigl3 PRIVATE glog::glog)
  target_link_libraries (tigl3_static PRIVATE glog::glog)
endif()

if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
   target_link_libraries (tigl3 PRIVATE rt)
   target_link_libraries (tigl3_static PRIVATE rt)
endif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
if (MSVC OR MINGW)
  target_link_libraries (tigl3 PRIVATE Shlwapi)
  target_link_libraries (tigl3_static PRIVATE Shlwapi)
endif(MSVC OR MINGW)

target_include_directories(tigl3_static PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/api>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/api>
    $<INSTALL_INTERFACE:include/tigl3>
)

add_library(tigl3_cpp INTERFACE)

foreach(directory ${TIGL_INCLUDES})
  target_include_directories(tigl3_cpp INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/${directory}>
    $<INSTALL_INTERFACE:include/tigl3/internal/${directory}>
  )
endforeach()
target_include_directories(tigl3_cpp INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
)

target_link_libraries(tigl3_cpp INTERFACE ${OpenCASCADE_LIBRARIES} Boost::boost Boost::disable_autolinking)

if (TARGET glog::glog)
  target_link_libraries (tigl3_cpp INTERFACE glog::glog)
endif()

# cmake configuration export
set(CMAKE_INSTALL_INCLUDE_DIR "include")
set(CONFIG_INSTALL_DIR "${CMAKE_INSTALL_LIBDIR}/cmake/tigl3")
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
  "${CMAKE_CURRENT_BINARY_DIR}/tigl3-config-version.cmake"
  VERSION ${TIGL_VERSION}
  COMPATIBILITY AnyNewerVersion
)


file(GENERATE
  OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/tigl3-config.cmake"
  INPUT  "${PROJECT_SOURCE_DIR}/cmake/tigl-config.cmake.in"
)

install (EXPORT tigl3-targets DESTINATION ${CONFIG_INSTALL_DIR})

install (FILES
  "${CMAKE_CURRENT_BINARY_DIR}/tigl3-config.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/tigl3-config-version.cmake"
  DESTINATION ${CONFIG_INSTALL_DIR})

# --------------------- Install TiGL CPP Bindings ----------------- #

if (TIGL_BINDINGS_INSTALL_CPP)
    
    configure_file(
        "${PROJECT_SOURCE_DIR}/cmake/tigl_cpp-config.cmake.in"
        "${CMAKE_CURRENT_BINARY_DIR}/tigl3_cpp-config.cmake"
        @ONLY
    )
    
    install (TARGETS tigl3_static
             EXPORT tigl3_cpp-targets
             RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
             LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
             ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    )

    install(TARGETS tigl3_cpp EXPORT tigl3_cpp-targets COMPONENT cpp_bindings)
    install (EXPORT tigl3_cpp-targets DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/tigl3_cpp)
    
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/tigl_config.h
        DESTINATION include/tigl3/internal COMPONENT cpp_bindings)
    
    install(DIRECTORY .
        DESTINATION include/tigl3/internal
        COMPONENT cpp_bindings
        FILES_MATCHING  PATTERN "*.h"
        PATTERN "*.hpp"
        PATTERN "*.hxx"
        PATTERN "api/*" EXCLUDE
    )

    install(FILES api/tigl_internal.h DESTINATION include/tigl3/internal/api)
    
    install (FILES
      "${CMAKE_CURRENT_BINARY_DIR}/tigl3_cpp-config.cmake"
      DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/tigl3_cpp)
    
    write_basic_package_version_file(
      "${CMAKE_CURRENT_BINARY_DIR}/tigl3_cpp-config-version.cmake"
      VERSION ${TIGL_VERSION}
      COMPATIBILITY AnyNewerVersion
    )

endif (TIGL_BINDINGS_INSTALL_CPP)

AddToCheckstyle()
