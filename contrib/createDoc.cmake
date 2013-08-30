find_package(Doxygen)
if(DOXYGEN_FOUND)
	configure_file(${PROJECT_SOURCE_DIR}/doc/Doxyfile.in ${PROJECT_BINARY_DIR}/doc/Doxyfile @ONLY)
	configure_file(${PROJECT_SOURCE_DIR}/doc/footer.html ${PROJECT_BINARY_DIR}/doc/footer.html @ONLY)
	add_custom_command(
		OUTPUT ${PROJECT_BINARY_DIR}/doc/html/index.html
		OUTPUT ${PROJECT_BINARY_DIR}/doc/latex/refman.tex
		DEPENDS ${PROJECT_SOURCE_DIR}/src/tigl.h
		COMMAND ${DOXYGEN_EXECUTABLE}
		ARGS ${PROJECT_BINARY_DIR}/doc/Doxyfile
	)

	add_custom_target(html
		DEPENDS ${PROJECT_BINARY_DIR}/doc/html/index.html
	)
	
	install(DIRECTORY ${PROJECT_BINARY_DIR}/doc/html
			DESTINATION	share/doc/tigl
			COMPONENT docu
			OPTIONAL)
			
		# create start menu entries
		SET(CPACK_NSIS_CREATE_ICONS_EXTRA ${CPACK_NSIS_CREATE_ICONS_EXTRA} "
	      CreateShortCut \\\"$SMPROGRAMS\\\\$STARTMENU_FOLDER\\\\Reference.lnk\\\" \\\"$INSTDIR\\\\share\\\\doc\\\\tigl\\\\html\\\\index.html\\\"
	    ")
	    SET(CPACK_NSIS_DELETE_ICONS_EXTRA ${CPACK_NSIS_DELETE_ICONS_EXTRA} "
	      !insertmacro MUI_STARTMENU_GETFOLDER Application $MUI_TEMP
	      Delete \\\"$SMPROGRAMS\\\\$MUI_TEMP\\\\Reference.lnk\\\"
	    ")			

	find_program(LATEX pdflatex)

	if(LATEX)
		add_custom_command(
			OUTPUT ${PROJECT_BINARY_DIR}/doc/tiglRef.pdf
			DEPENDS ${PROJECT_BINARY_DIR}/doc/latex/refman.tex
			WORKING_DIRECTORY ${PROJECT_BINARY_DIR}/doc/latex/		
			COMMAND ${LATEX}
			ARGS ${PROJECT_BINARY_DIR}/doc/latex/refman.tex
			COMMAND ${CMAKE_COMMAND}
			ARGS -E copy ${PROJECT_BINARY_DIR}/doc/latex/refman.pdf ${PROJECT_BINARY_DIR}/doc/tiglRef.pdf
		)
		
		add_custom_command(
			OUTPUT ${PROJECT_BINARY_DIR}/doc/tiglGuide.pdf
			DEPENDS ${PROJECT_SOURCE_DIR}/doc/tiglGuide/tiglGuide.tex
			WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/doc/tiglGuide/
			COMMAND ${LATEX}
			ARGS tiglGuide.tex --output-directory=${PROJECT_BINARY_DIR}/doc/
			# run latex twice to get references right (normal use case of latex, don't ask why)
		    COMMAND ${LATEX}
			ARGS tiglGuide.tex --output-directory=${PROJECT_BINARY_DIR}/doc/
		)

		add_custom_target(pdf
			COMMENT "Generating PDF reference documentation with latex" VERBATIM 
			DEPENDS ${PROJECT_BINARY_DIR}/doc/tiglRef.pdf
		)

		add_custom_target(guide
			COMMENT "Generating PDF Guide for TIGL with latex" VERBATIM 
			DEPENDS ${PROJECT_BINARY_DIR}/doc/tiglGuide.pdf
		)
		add_custom_target(doc
			DEPENDS html pdf guide
			COMMENT "Generating API documentation with Doxygen" VERBATIM 
		)
		
		install(FILES ${PROJECT_BINARY_DIR}/doc/tiglRef.pdf 
				DESTINATION	share/doc/tigl
				COMPONENT docu
				OPTIONAL)
				
		install(FILES ${PROJECT_BINARY_DIR}/doc/tiglGuide.pdf 
				DESTINATION	share/doc/tigl
				COMPONENT docu
				OPTIONAL)
			
		# create start menu entries
		SET(CPACK_NSIS_CREATE_ICONS_EXTRA ${CPACK_NSIS_CREATE_ICONS_EXTRA} "
	      CreateShortCut \\\"$SMPROGRAMS\\\\$STARTMENU_FOLDER\\\\TIGL-Reference.lnk\\\" \\\"$INSTDIR\\\\share\\\\doc\\\\tigl\\\\tiglRef.pdf\\\"
	    ")
	    SET(CPACK_NSIS_DELETE_ICONS_EXTRA ${CPACK_NSIS_DELETE_ICONS_EXTRA} "
	      !insertmacro MUI_STARTMENU_GETFOLDER Application $MUI_TEMP
	      Delete \\\"$SMPROGRAMS\\\\$MUI_TEMP\\\\TIGL-Reference.lnk\\\"
	    ")
	    SET(CPACK_NSIS_CREATE_ICONS_EXTRA ${CPACK_NSIS_CREATE_ICONS_EXTRA} "
	      CreateShortCut \\\"$SMPROGRAMS\\\\$STARTMENU_FOLDER\\\\TIGL-Guide.lnk\\\" \\\"$INSTDIR\\\\share\\\\doc\\\\tigl\\\\tiglGuide.pdf\\\"
	    ")
	    SET(CPACK_NSIS_DELETE_ICONS_EXTRA ${CPACK_NSIS_DELETE_ICONS_EXTRA} "
	      !insertmacro MUI_STARTMENU_GETFOLDER Application $MUI_TEMP
	      Delete \\\"$SMPROGRAMS\\\\$MUI_TEMP\\\\TIGL-Guide.lnk\\\"
	    ")
		
		
	else()
		add_custom_target(doc
			DEPENDS html
			COMMENT "Generating API documentation with Doxygen" VERBATIM 
		)
	endif()
endif(DOXYGEN_FOUND)
