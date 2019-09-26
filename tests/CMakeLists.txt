add_subdirectory(unittests)

# add confidential tests
OPTION(BUILD_TIGL_CONFIDENTIAL_TESTS "Build tests on confidential data" OFF)
if (BUILD_TIGL_CONFIDENTIAL_TESTS)
    add_subdirectory(conftests)
endif()
