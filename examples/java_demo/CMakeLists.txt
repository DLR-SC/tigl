# tigl demo cmake project
# author: Martin Siggel

install(FILES 
    JavaDemo.java
    DESTINATION share/doc/tigl3/examples
    COMPONENT docu
)

if (TIGL_BINDINGS_JAVA)
	SET(TIGL_JAR "${PROJECT_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR}/tigl-${TIGL_VERSION}.jar")

	set(TIGL_LIB_PATH ${PROJECT_BINARY_DIR}/lib)
	
	configure_file(${CMAKE_CURRENT_SOURCE_DIR}/build.gradle.in ${CMAKE_CURRENT_BINARY_DIR}/build.gradle)
	configure_file(${CMAKE_CURRENT_SOURCE_DIR}/gradle.properties ${CMAKE_CURRENT_BINARY_DIR}/gradle.properties COPYONLY )
	
	ADD_CUSTOM_TARGET(
		tigl-java-demo 
		DEPENDS ${TIGL_JAR}
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
		COMMAND   ${Java_JAVA_EXECUTABLE} -classpath ${PROJECT_SOURCE_DIR}/bindings/java/gradle/wrapper/gradle-wrapper.jar org.gradle.wrapper.GradleWrapperMain build run
	)

endif()
