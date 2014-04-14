add_subdirectory(c_demo)

install(FILES 
    ${PROJECT_SOURCE_DIR}/tests/TestData/simpletest.cpacs.xml
    README.txt
    DESTINATION share/doc/tigl/examples
    COMPONENT docu
)
