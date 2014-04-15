add_subdirectory(c_demo)
add_subdirectory(matlab_demo)
add_subdirectory(python_demo)

install(FILES 
    ${PROJECT_SOURCE_DIR}/tests/TestData/simpletest.cpacs.xml
    README.txt
    DESTINATION share/doc/tigl/examples
    COMPONENT docu
)
