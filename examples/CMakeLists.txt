add_subdirectory(c_demo)
add_subdirectory(matlab_demo)
add_subdirectory(python_demo)

install(FILES 
    ${PROJECT_SOURCE_DIR}/tests/TestData/simpletest.cpacs.xml
    DESTINATION share/doc/tigl/examples
    COMPONENT docu
)

install(FILES 
    README.md
    DESTINATION share/doc/tigl/examples
    COMPONENT docu
    RENAME README.txt
)
