# tigl_get_cpacs_schema_version(<schema_file> <out_var>)
#
# Determines the CPACS version of the given CPACS schema file and stores it in <out_var>.
#
# The schema has no machine-readable version attribute, so the highest version found in the
# release notes section titles (<ddue:title>CPACS x.y[.z]</ddue:title>) is used.
# Titles inside XML comments are ignored.
# The schema file is added as configure dependency, i.e. CMake re-runs if it changes.
function(tigl_get_cpacs_schema_version SCHEMA_FILE OUT_VAR)
    if (NOT EXISTS "${SCHEMA_FILE}" OR IS_DIRECTORY "${SCHEMA_FILE}")
        message(FATAL_ERROR "CPACS schema file not found: ${SCHEMA_FILE}. "
                            "It is required to determine the CPACS version TiGL is built against.")
    endif()

    set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${SCHEMA_FILE}")

    file(READ "${SCHEMA_FILE}" _content)
    # Remove XML comments. XML forbids "--" inside comments, so this matches exactly one comment.
    string(REGEX REPLACE "<!--([^-]|-[^-])*-->" "" _content "${_content}")

    string(REGEX MATCHALL "<ddue:title>CPACS [0-9]+\\.[0-9]+(\\.[0-9]+)?</ddue:title>" _titles "${_content}")
    set(_cpacs_version "")
    foreach(_title IN LISTS _titles)
        string(REGEX REPLACE "<ddue:title>CPACS ([0-9.]+)</ddue:title>" "\\1" _version "${_title}")
        if (_cpacs_version STREQUAL "" OR _version VERSION_GREATER _cpacs_version)
            set(_cpacs_version "${_version}")
        endif()
    endforeach()

    if (_cpacs_version STREQUAL "")
        message(FATAL_ERROR "Could not determine the CPACS version from ${SCHEMA_FILE}. "
                            "Expected release notes section titles of the form '<ddue:title>CPACS x.y.z</ddue:title>'.")
    endif()

    set(${OUT_VAR} "${_cpacs_version}" PARENT_SCOPE)
endfunction()
