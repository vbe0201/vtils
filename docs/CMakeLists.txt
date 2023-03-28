find_package(Doxygen REQUIRED)
find_package(Sphinx REQUIRED)

# Gather all public headers from the target header set.
# This is a list taht we will convert to a space-separated string.
get_target_property(
    ${PROJECT_NAME}_PUBLIC_HEADERS
    ${PROJECT_NAME}
    HEADER_SET_HEADERS
)
string(REPLACE ";" " " ${PROJECT_NAME}_PUBLIC_HEADERS_STR "${${PROJECT_NAME}_PUBLIC_HEADERS}")

set(DOXYGEN_INPUT ${${PROJECT_NAME}_PUBLIC_HEADERS_STR})
set(DOXYGEN_OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/doxygen)
set(DOXYGEN_INDEX_FILE ${DOXYGEN_OUTPUT}/xml/index.xml)
set(DOXYFILE_IN ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in)
set(DOXYFILE_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

# Generate the real Doxygen configuration with substituted variables.
configure_file(${DOXYFILE_IN} ${DOXYFILE_OUT} @ONLY)

# Compose a Doxygen target for building library documentation.
file(MAKE_DIRECTORY ${DOXYGEN_OUTPUT})
add_custom_command(
    OUTPUT ${DOXYGEN_INDEX_FILE}
    DEPENDS ${PROJECT_NAME} ${${PROJECT_NAME}_PUBLIC_HEADERS}
    COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYFILE_OUT}
    MAIN_DEPENDENCY ${DOXYFILE_IN} ${DOXYFILE_OUT}
    COMMENT "Generate Doxygen documentation"
    VERBATIM
)
add_custom_target(${PROJECT_NAME}_Doxygen ALL DEPENDS ${DOXYGEN_INDEX_FILE})

set(SPHINX_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/source)
set(SPHINX_BUILD ${CMAKE_CURRENT_BINARY_DIR}/sphinx)
set(SPHINX_INDEX_FILE ${SPHINX_BUILD}/index.html)

# Compose a Sphinx target to build the public-facing documentation.
# This will only be regenerated after Doxygen was re-run, the doc files
# have been updated or when the Sphinx config has been changed.
add_custom_command(
    OUTPUT ${SPHINX_INDEX_FILE}
    COMMAND
    ${SPHINX_EXECUTABLE} -b html
    # Tell Breathe where to find the Doxygen output.
    -Dbreathe_projects.${PROJECT_NAME}=${DOXYGEN_OUTPUT}/xml
    ${SPHINX_SOURCE} ${SPHINX_BUILD}
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    DEPENDS
    # Documentation files that need to be tracked go here.
    ${DOXYGEN_INDEX_FILE}
    MAIN_DEPENDENCY ${SPHINX_SOURCE}/conf.py
    COMMENT "Sphinx project documentation build"
)
add_custom_target(${PROJECT_NAME}_Sphinx ALL DEPENDS ${SPHINX_INDEX_FILE})

# Install the documentation when needed.
if(VTILS_OPT_INSTALL)
    # TODO
endif()
