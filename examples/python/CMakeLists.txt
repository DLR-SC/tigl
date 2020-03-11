# tigl demo cmake project
# author: Martin Siggel

install(FILES 
    api_basic_demo.py
    DESTINATION share/doc/tigl3/examples
    RENAME python_demo.py
    COMPONENT docu
)


install(FILES
        create_space_ship.py
        DESTINATION share/doc/tigl3/examples
        COMPONENT docu
        )


install(FILES
        data/empty.cpacs3.xml
        DESTINATION share/doc/tigl3/examples/data
        COMPONENT docu
        )
