# TIGL Library

find_package(Threads REQUIRED)
include(tiglmacros)

configure_file (
  "${CMAKE_CURRENT_SOURCE_DIR}/api/tigl_version.h.in"
  "${CMAKE_CURRENT_BINARY_DIR}/api/tigl_version.h"
)

# add all subdirectories to include path
set(TIGL_INCLUDES ${CMAKE_CURRENT_SOURCE_DIR})
subdirlist(SUBDIRS ${CMAKE_CURRENT_SOURCE_DIR})
foreach(subdir ${SUBDIRS})
  set(TIGL_INCLUDES ${TIGL_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR}/${subdir})
endforeach()

include_directories(${OpenCASCADE_INCLUDE_DIR}
                    ${TIXI_INCLUDE_DIRS}
                    ${CMAKE_CURRENT_BINARY_DIR}
                    ${CMAKE_CURRENT_BINARY_DIR}/api
                    ${TIGL_INCLUDES}
)

# boost
set  (BOOST_DIR ${PROJECT_SOURCE_DIR}/thirdparty/boost_1_67_0)
include_directories(${BOOST_DIR})
set (BOOST_SRC
    ${BOOST_DIR}/libs/atomic/src/lockpool.cpp
    ${BOOST_DIR}/libs/chrono/src/chrono.cpp
    ${BOOST_DIR}/libs/chrono/src/process_cpu_clocks.cpp
    ${BOOST_DIR}/libs/chrono/src/thread_clock.cpp
    ${BOOST_DIR}/libs/date_time/src/gregorian/date_generators.cpp
    ${BOOST_DIR}/libs/date_time/src/gregorian/gregorian_types.cpp
    ${BOOST_DIR}/libs/date_time/src/gregorian/greg_month.cpp
    ${BOOST_DIR}/libs/date_time/src/gregorian/greg_weekday.cpp
    ${BOOST_DIR}/libs/date_time/src/posix_time/posix_time_types.cpp
    ${BOOST_DIR}/libs/exception/src/clone_current_exception_non_intrusive.cpp
    ${BOOST_DIR}/libs/filesystem/src/codecvt_error_category.cpp
    ${BOOST_DIR}/libs/filesystem/src/operations.cpp
    ${BOOST_DIR}/libs/filesystem/src/path.cpp
    ${BOOST_DIR}/libs/filesystem/src/path_traits.cpp
    ${BOOST_DIR}/libs/filesystem/src/portability.cpp
    ${BOOST_DIR}/libs/filesystem/src/unique_path.cpp
    ${BOOST_DIR}/libs/filesystem/src/utf8_codecvt_facet.cpp
    ${BOOST_DIR}/libs/filesystem/src/windows_file_codecvt.cpp
    ${BOOST_DIR}/libs/system/src/error_code.cpp
    ${BOOST_DIR}/libs/thread/src/future.cpp
)

add_definitions(-DBOOST_THREAD_BUILD_LIB)
if(WIN32)
    add_definitions(-DBOOST_THREAD_WIN32)
    set (BOOST_SRC ${BOOST_SRC}
        ${BOOST_DIR}/libs/thread/src/win32/thread.cpp
        ${BOOST_DIR}/libs/thread/src/win32/thread_primitives.cpp
        ${BOOST_DIR}/libs/thread/src/win32/tss_dll.cpp
        ${BOOST_DIR}/libs/thread/src/win32/tss_pe.cpp
    )
else()
    add_definitions(-DBOOST_THREAD_POSIX)
    set (BOOST_SRC ${BOOST_SRC}
        ${BOOST_DIR}/libs/thread/src/pthread/once.cpp
        ${BOOST_DIR}/libs/thread/src/pthread/thread.cpp
    )
endif()

file(GLOB_RECURSE TIGL_SRC
    "*.h"
    "*.hxx"
    "*.cpp"
    "*.cxx"
)
set(TIGL_SRC ${TIGL_SRC} ${BOOST_SRC})

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

if(MSVC)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS=1 /wd4355 -DTIGL_INTERNAL_EXPORTS)
endif(MSVC)

if(CMAKE_COMPILER_IS_GNUCC)
    # gcc 4.3.4 creates wrong code for boost::hash if compiled with optimizations
    SET_SOURCE_FILES_PROPERTIES(CTiglIntersectionCalculation.cpp PROPERTIES COMPILE_FLAGS -O0)
endif(CMAKE_COMPILER_IS_GNUCC)

# object library containing just the compiled sources
add_library(tigl3_objects OBJECT ${TIGL_SRC})
set_property(TARGET tigl3_objects PROPERTY POSITION_INDEPENDENT_CODE ON) # needed for shared libraries
target_compile_definitions(tigl3_objects PRIVATE -Dtigl3_EXPORTS)

# tigl dll/so library
add_library(tigl3 SHARED $<TARGET_OBJECTS:tigl3_objects>)

if(GLOG_FOUND)
  include_directories(${GLOG_INCLUDE_DIR})
  target_link_libraries (TIGL ${GLOG_LIBRARIES})
endif(GLOG_FOUND)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  add_definitions(-DDEBUG)
  if(DEBUG_EXTENSIVE)
      add_definitions(-DDEBUG_GUIDED_SURFACE_CREATION)
  endif()
endif()

set(TIGL_LIBRARIES ${OpenCASCADE_LIBRARIES} ${TIXI_LIBRARIES} ${VLD_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  set(TIGL_LIBRARIES  ${TIGL_LIBRARIES} rt)
endif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
if (WIN32)
  set(TIGL_LIBRARIES  ${TIGL_LIBRARIES} Shlwapi)
endif(WIN32)

target_link_libraries (tigl3 ${TIGL_LIBRARIES})
set_target_properties( tigl3 PROPERTIES VERSION ${TIGL_VERSION_MINOR}.${TIGL_VERSION_PATCH})
set_target_properties( tigl3 PROPERTIES SOVERSION "${TIGL_VERSION_MINOR}")

install (TARGETS tigl3
         EXPORT tigl-targets
         RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
         LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
         ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
         )
         
install (EXPORT tigl-targets DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/tigl3)
         
install (FILES api/tigl.h ${CMAKE_CURRENT_BINARY_DIR}/api/tigl_version.h
         DESTINATION include/tigl3
         COMPONENT headers)

# static lib for tiglviewer 
add_library(tigl3_static $<TARGET_OBJECTS:tigl3_objects>)
# this does no actual linking but keeps track of the dependencies
target_link_libraries (tigl3_static ${TIGL_LIBRARIES})

# cmake configuration export
set(CMAKE_INSTALL_INCLUDE_DIR "include")
set(CONFIG_INSTALL_DIR "${CMAKE_INSTALL_LIBDIR}/cmake/tigl3")
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
  "${CMAKE_CURRENT_BINARY_DIR}/tigl-config-version.cmake"
  VERSION ${TIGL_VERSION}
  COMPATIBILITY AnyNewerVersion
)

configure_package_config_file (${PROJECT_SOURCE_DIR}/cmake/tigl-config.cmake.in 
    "${CMAKE_CURRENT_BINARY_DIR}/tigl-config.cmake"
    INSTALL_DESTINATION ${CONFIG_INSTALL_DIR}
    PATH_VARS CMAKE_INSTALL_INCLUDE_DIR CONFIG_INSTALL_DIR
)
install (FILES
  "${CMAKE_CURRENT_BINARY_DIR}/tigl-config.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/tigl-config-version.cmake"
  DESTINATION ${CONFIG_INSTALL_DIR})

AddToCheckstyle()
