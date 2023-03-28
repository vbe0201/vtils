include(FindPackageHandleStandardArgs)

find_program(SPHINX_EXECUTABLE
        NAMES sphinx-build
        DOC "Sphinx documentation engine build tool"
        )

find_package_handle_standard_args(Sphinx
        REQUIRED_VARS SPHINX_EXECUTABLE
        )

mark_as_advanced(SPHINX_EXECUTABLE)
