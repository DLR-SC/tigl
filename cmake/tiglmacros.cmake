function(AddToCheckstyle)
  if(PYTHONINTERP_FOUND)
    get_filename_component(dir ${CMAKE_CURRENT_SOURCE_DIR} NAME)

    add_custom_command(
        OUTPUT ${dir}_checkstyle-report
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/filefilter.txt
        WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
        COMMAND ${PYTHON_EXECUTABLE}
        ARGS ${PROJECT_SOURCE_DIR}/thirdparty/nsiqcppstyle/nsiqcppstyle.py -o ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR}
    )

    add_custom_command(
        OUTPUT ${dir}_checkstyle-xml
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/filefilter.txt
        WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
        COMMAND ${PYTHON_EXECUTABLE}
        ARGS ${PROJECT_SOURCE_DIR}/thirdparty/nsiqcppstyle/nsiqcppstyle.py --ci -o ${CMAKE_CURRENT_BINARY_DIR} --output=xml ${CMAKE_CURRENT_SOURCE_DIR}
    )

    add_custom_command(
        OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/filefilter.txt
        COMMAND ${CMAKE_COMMAND}
        ARGS -E copy ${PROJECT_SOURCE_DIR}/thirdparty/nsiqcppstyle/filefilter.txt ${CMAKE_CURRENT_SOURCE_DIR}/
    )

    add_custom_target(
        checkstyle_${dir}
        DEPENDS ${dir}_checkstyle-report
    )

    add_custom_target(
        checkstylexml_${dir}
        DEPENDS ${dir}_checkstyle-xml
    )


    add_dependencies(checkstyle checkstyle_${dir})
    add_dependencies(checkstylexml checkstylexml_${dir})

  endif(PYTHONINTERP_FOUND)
endfunction()

# gets the list of all subdirectories relative to curdir
# example use:
#   SUBDIRLIST(SUBDIRS ${MY_CURRENT_DIR})
macro(SUBDIRLIST result curdir)
  file(GLOB children RELATIVE ${curdir} ${curdir}/*)
  set(dirlist "")
  foreach(child ${children})
    if(IS_DIRECTORY ${curdir}/${child})
        set(dirlist ${dirlist} ${child})
    endif()
  endforeach()
  set(${result} ${dirlist})
endmacro()

# Concatenate files
#
# Parameters :
# IN - input file)
# OUT - output file
FUNCTION(CAT IN OUT)
    FILE(READ ${IN} CONTENTS)
    FILE(APPEND ${OUT} "${CONTENTS}")
ENDFUNCTION(CAT IN OUT)
