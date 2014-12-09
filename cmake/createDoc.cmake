find_package(Doxygen 1.8.0)
if(DOXYGEN_FOUND AND PYTHONINTERP_FOUND)

    file(GLOB_RECURSE DOC_MD_SRC  "${PROJECT_SOURCE_DIR}/doc/*.md")


    # convert ChangeLog to Markdown for usage in doxygen 
    add_custom_command(
        OUTPUT ${PROJECT_BINARY_DIR}/doc/ChangeLog.md
        DEPENDS ${PROJECT_SOURCE_DIR}/ChangeLog
        COMMAND ${PYTHON_EXECUTABLE} ${PROJECT_SOURCE_DIR}/misc/createChangeLog/changeLogToMD.py -i ${PROJECT_SOURCE_DIR}/ChangeLog -o ${PROJECT_BINARY_DIR}/doc/ChangeLog.md
    )

    configure_file(${PROJECT_SOURCE_DIR}/doc/Doxyfile.in ${PROJECT_BINARY_DIR}/doc/Doxyfile @ONLY)
    configure_file(${PROJECT_SOURCE_DIR}/doc/footer.html ${PROJECT_BINARY_DIR}/doc/footer.html @ONLY)
    add_custom_command(
        OUTPUT ${PROJECT_BINARY_DIR}/doc/html/index.html
        OUTPUT ${PROJECT_BINARY_DIR}/doc/latex/refman.tex
        DEPENDS ${PROJECT_SOURCE_DIR}/src/api/tigl.h
        DEPENDS ${DOC_MD_SRC}
        DEPENDS ${PROJECT_BINARY_DIR}/doc/Doxyfile
        DEPENDS ${PROJECT_BINARY_DIR}/doc/ChangeLog.md
        COMMAND ${DOXYGEN_EXECUTABLE}
        ARGS ${PROJECT_BINARY_DIR}/doc/Doxyfile
    )

    add_custom_target(html
        DEPENDS ${PROJECT_BINARY_DIR}/doc/html/index.html
    )
    
    install(DIRECTORY ${PROJECT_BINARY_DIR}/doc/html
            DESTINATION    share/doc/tigl
            COMPONENT docu
            OPTIONAL)

        # create start menu entries
        SET(CPACK_NSIS_CREATE_ICONS_EXTRA ${CPACK_NSIS_CREATE_ICONS_EXTRA} "
        CreateShortCut \\\"$SMPROGRAMS\\\\$STARTMENU_FOLDER\\\\Documentation.lnk\\\" \\\"$INSTDIR\\\\share\\\\doc\\\\tigl\\\\html\\\\index.html\\\"
        ")
        SET(CPACK_NSIS_DELETE_ICONS_EXTRA ${CPACK_NSIS_DELETE_ICONS_EXTRA} "
          !insertmacro MUI_STARTMENU_GETFOLDER Application $MUI_TEMP
          Delete \\\"$SMPROGRAMS\\\\$MUI_TEMP\\\\Documentation.lnk\\\"
        ")            

    find_program(LATEX pdflatex)

    if(LATEX)
        # TIGL Reference PDF File
        add_custom_command(
            OUTPUT ${PROJECT_BINARY_DIR}/doc/tiglRef.pdf
            DEPENDS ${PROJECT_BINARY_DIR}/doc/latex/refman.tex
            WORKING_DIRECTORY ${PROJECT_BINARY_DIR}/doc/latex/        
            COMMAND ${LATEX}
            ARGS -quiet ${PROJECT_BINARY_DIR}/doc/latex/refman.tex
            # run latex twice to get references right (normal use case of latex, don't ask why)
            COMMAND ${LATEX}
            ARGS -quiet ${PROJECT_BINARY_DIR}/doc/latex/refman.tex
            COMMAND ${CMAKE_COMMAND}
            ARGS -E copy ${PROJECT_BINARY_DIR}/doc/latex/refman.pdf ${PROJECT_BINARY_DIR}/doc/tiglRef.pdf
        )
        
        # TIGL Guide
        # create latex build directory
        add_custom_command(
            OUTPUT ${PROJECT_BINARY_DIR}/doc/tiglGuide/
            DEPENDS ${PROJECT_SOURCE_DIR}/doc/tiglGuide/tiglGuide.tex 
            COMMAND ${CMAKE_COMMAND}
            ARGS -E copy_directory ${PROJECT_SOURCE_DIR}/doc/tiglGuide/ ${PROJECT_BINARY_DIR}/doc/tiglGuide
        )
        # create pdf file
        add_custom_command(
            OUTPUT ${PROJECT_BINARY_DIR}/doc/tiglGuide/tiglGuide.pdf
            DEPENDS ${PROJECT_SOURCE_DIR}/doc/tiglGuide/tiglGuide.tex ${PROJECT_BINARY_DIR}/doc/tiglGuide/
            WORKING_DIRECTORY ${PROJECT_BINARY_DIR}/doc/tiglGuide/        
            # create directory for latex build
            # build pdf
            COMMAND ${LATEX}
            ARGS -quiet tiglGuide.tex 
            # run latex twice to get references right (normal use case of latex, don't ask why)
            COMMAND ${LATEX}
            ARGS -quiet tiglGuide.tex
        )

        add_custom_target(pdf
            COMMENT "Generating PDF reference documentation with latex" VERBATIM 
            DEPENDS ${PROJECT_BINARY_DIR}/doc/tiglRef.pdf
        )

        add_custom_target(guide
            COMMENT "Generating PDF Guide for TIGL with latex" VERBATIM 
            DEPENDS ${PROJECT_BINARY_DIR}/doc/tiglGuide/tiglGuide.pdf
        )

        add_custom_target(doc
            DEPENDS html pdf guide
            COMMENT "Generating API documentation with Doxygen" VERBATIM 
        )
        
        install(FILES ${PROJECT_BINARY_DIR}/doc/tiglRef.pdf 
                DESTINATION    share/doc/tigl
                COMPONENT docu
                OPTIONAL)
                
            
        # create start menu entries
        SET(CPACK_NSIS_CREATE_ICONS_EXTRA ${CPACK_NSIS_CREATE_ICONS_EXTRA} "
          CreateShortCut \\\"$SMPROGRAMS\\\\$STARTMENU_FOLDER\\\\TiGL-Reference.lnk\\\" \\\"$INSTDIR\\\\share\\\\doc\\\\tigl\\\\tiglRef.pdf\\\"
        ")
        SET(CPACK_NSIS_DELETE_ICONS_EXTRA ${CPACK_NSIS_DELETE_ICONS_EXTRA} "
          !insertmacro MUI_STARTMENU_GETFOLDER Application $MUI_TEMP
          Delete \\\"$SMPROGRAMS\\\\$MUI_TEMP\\\\TiGL-Reference.lnk\\\"
        ")
        
    else()
        add_custom_target(doc
            DEPENDS html
            COMMENT "Generating API documentation with Doxygen" VERBATIM 
        )
    endif()
endif(DOXYGEN_FOUND AND PYTHONINTERP_FOUND)
