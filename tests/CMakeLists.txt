add_subdirectory(unittests)
if (BUILD_TIGL_CONFIDENTIAL_TESTS)
    add_subdirectory(conftests)
endif()

add_custom_target(test DEPENDS ${TEST_TARGETS})
