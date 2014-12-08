find_package(Java 1.7.0 COMPONENTS Development)

if (JAVA_FOUND)

	set(TIGLJAVA_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

	configure_file(${CMAKE_CURRENT_SOURCE_DIR}/build.gradle.in ${CMAKE_CURRENT_BINARY_DIR}/build.gradle)

	if(NOT WIN32)
		set(GRADLE_CMD "gradlew")
	else()
		set(GRADLE_CMD "gradlew.bat")	
	endif()

	file(GLOB_RECURSE TIGL_JAVA_SRC "*.java")

	SET(TIGL_JAR "${PROJECT_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR}/tigl-${TIGL_VERSION}.jar")
	
	add_custom_command(
		OUTPUT  ${TIGL_JAR}
		DEPENDS ${TIGL_JAVA_SRC}
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
		COMMAND   ${CMAKE_CURRENT_SOURCE_DIR}/${GRADLE_CMD}
		ARGS      "assemble" 
	)
	
	ADD_CUSTOM_TARGET(
		tigl-java ALL 
		DEPENDS ${TIGL_JAR}
	)
	
	install(FILES ${TIGL_JAR} DESTINATION share/tigl/java)

endif()

