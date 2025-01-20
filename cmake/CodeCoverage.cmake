# - Enable Code Coverage
#
# 2012-01-31, Lars Bilke
#
# USAGE:
# 1. Copy this file into your cmake modules path
# 2. Add the following line to your CMakeLists.txt:
#      INCLUDE(CodeCoverage)
# 
# 3. Use the function SETUP_TARGET_FOR_COVERAGE to create a custom make target
#    which runs your test executable and produces a lcov code coverage report.
#

OPTION(TIGL_COVERAGE_GENHTML "Use Genhtml to generate htmls from gcov output" OFF)

# Check prereqs
FIND_PROGRAM( GCOV_PATH gcov )
FIND_PROGRAM( LCOV_PATH lcov )

IF(TIGL_COVERAGE_GENHTML)
  FIND_PROGRAM( GENHTML_PATH genhtml )
ENDIF()
FIND_PROGRAM( GCOVR_PATH gcovr PATHS ${PROJECT_SOURCE_DIR}/tests)

IF(NOT GCOV_PATH)
        MESSAGE(FATAL_ERROR "gcov not found! Aborting...")
ENDIF() # NOT GCOV_PATH

IF(NOT CMAKE_COMPILER_IS_GNUCXX AND NOT "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
        MESSAGE(FATAL_ERROR "Compiler is not GNU gcc or clang! Aborting...")
ENDIF() # NOT CMAKE_COMPILER_IS_GNUCXX AND NOT CLANG

IF ( NOT CMAKE_BUILD_TYPE STREQUAL "Debug" )
  MESSAGE( WARNING "Code coverage results with an optimised (non-Debug) build may be misleading" )
ENDIF() # NOT CMAKE_BUILD_TYPE STREQUAL "Debug"


# Setup compiler options
IF(CMAKE_COMPILER_IS_GNUCXX)
  ADD_DEFINITIONS(-fprofile-arcs -ftest-coverage)
  LINK_LIBRARIES(gcov)
ENDIF()

IF("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
  ADD_DEFINITIONS("--coverage")
  SET( CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} --coverage" )
  SET (CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS_INIT} --coverage" )
ENDIF()

# Param _targetname     The name of new the custom make target
# Param _testrunner     The name of the target which runs the tests
# Param _testdir        Relative path to the test directory from PROJECT_SOURCE_DIR
# Param _outputname     lcov output is generated as _outputname.info
#                       HTML report is generated in _outputname/index.html
# Optional fourth parameter is passed as arguments to _testrunner
#   Pass them in list form, e.g.: "-j;2" for -j 2
FUNCTION(SETUP_TARGET_FOR_COVERAGE _targetname _testrunner _testdir _outputname)

        IF(NOT LCOV_PATH)
                MESSAGE(FATAL_ERROR "lcov not found! Aborting...")
        ENDIF() # NOT LCOV_PATH

        IF(TIGL_COVERAGE_GENHTML)
        IF(NOT GENHTML_PATH)
                MESSAGE(FATAL_ERROR "genhtml not found! Aborting...")
        ENDIF() # NOT GENHTML_PATH
        ENDIF() # TIGL_COVERAGE_GENHTML
        
        # copy testdata
        file(COPY ${PROJECT_SOURCE_DIR}/${_testdir}/TestData DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
        if (BUILD_TIGL_CONFIDENTIAL_TESTS)
            file(COPY ${PROJECT_SOURCE_DIR}/tests/conftests/TestData DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
        endif()

        # Setup target
        ADD_CUSTOM_TARGET(${_targetname}
        
                # Cleanup lcov
                ${LCOV_PATH} --directory .. --zerocounters

                # Run tests
                COMMAND ${_testrunner} ${ARGV3}

                # Capturing lcov counters and generating report
                COMMAND ${LCOV_PATH} --directory ../.. --capture --ignore-errors mismatch --output-file ${_outputname}_all.info
                COMMAND ${LCOV_PATH} --remove ${_outputname}_all.info '${PROJECT_SOURCE_DIR}/${_testdir}/*' '${PROJECT_SOURCE_DIR}/tests/common/*' '${PROJECT_SOURCE_DIR}/thirdparty/**' '/usr/*' '*oce*' '*build*' '*tixi3*' '${PROJECT_SOURCE_DIR}/src/generated/*' --output-file ${_outputname}.info

                IF(TIGL_COVERAGE_GENHTML)
                        COMMAND ${GENHTML_PATH} -o ../../${_outputname} ${_outputname}.info
                ENDIF()
                
                COMMAND ${CMAKE_COMMAND} -E remove ${_outputname}_all.info 

                USES_TERMINAL
                WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/${_testdir}
                COMMENT "Resetting code coverage counters to zero.\nProcessing code coverage counters and generating report."
        )

        # Show info where to find the report
        ADD_CUSTOM_COMMAND(TARGET ${_targetname} POST_BUILD
                COMMAND ;
                COMMENT "Open ./${_outputname}/index.html in your browser to view the coverage report."
        )

ENDFUNCTION() # SETUP_TARGET_FOR_COVERAGE

# Param _targetname     The name of new the custom make target
# Param _testrunner     The name of the target which runs the tests
# Param _outputname     cobertura output is generated as _outputname.xml
# Optional fourth parameter is passed as arguments to _testrunner
#   Pass them in list form, e.g.: "-j;2" for -j 2
FUNCTION(SETUP_TARGET_FOR_COVERAGE_COBERTURA _targetname _testrunner _outputname)

        IF(NOT GCOVR_PATH)
                MESSAGE(FATAL_ERROR "gcovr not found! Aborting...")
        ENDIF() # NOT GCOVR_PATH
        
        ADD_CUSTOM_TARGET(${_targetname}
                # Run tests
                COMMAND ${_testrunner} ${ARGV3}
                WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/tests/unittests
                COMMENT "Running coverage tests."
        )
        
        ADD_CUSTOM_COMMAND(TARGET ${_targetname} POST_BUILD
                # Running gcovr
                COMMAND ${GCOVR_PATH} -x -r ${CMAKE_SOURCE_DIR} -e '${CMAKE_SOURCE_DIR}/tests/' -e '${CMAKE_SOURCE_DIR}/thirdparty/' -e '${CMAKE_SOURCE_DIR}/build/' -o ${_outputname}.xml
                WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
                COMMENT "Running gcovr to produce Cobertura code coverage report. ${_testrunner} ${ARGV3}"
        )

        # Show info where to find the report
        ADD_CUSTOM_COMMAND(TARGET ${_targetname} POST_BUILD
                COMMAND ;
                COMMENT "Cobertura code coverage report saved in ${_outputname}.xml."
        )

ENDFUNCTION() # SETUP_TARGET_FOR_COVERAGE_COBERTURA 
