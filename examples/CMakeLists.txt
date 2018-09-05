add_subdirectory(c_demo)
add_subdirectory(matlab_demo)
add_subdirectory(python)
add_subdirectory(java_demo)

install(FILES 
    ${PROJECT_SOURCE_DIR}/tests/unittests/TestData/simpletest.cpacs.xml
    DESTINATION share/doc/tigl3/examples
    COMPONENT docu
)

install(FILES 
    README.md
    DESTINATION share/doc/tigl3/examples
    COMPONENT docu
    RENAME README.txt
)
