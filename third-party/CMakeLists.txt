include(FetchContent)

macro(check_submodules)
    # Read and extract all modules from .gitmodules.
    file(READ "${CMAKE_SOURCE_DIR}/.gitmodules" __VTILS_GITMODULES)
    string(REGEX MATCHALL "path *= *[^\t\r\n]*" __VTILS_GITMODULES ${__VTILS_GITMODULES})

    # Verify that each module has a .git directory in its path,
    # i.e. ensure that all submodules are initialized and available.
    foreach(module ${__VTILS_GITMODULES})
        string(REGEX REPLACE "path *= *" "" module ${module})
        if(NOT EXISTS "${CMAKE_SOURCE_DIR}/${module}/.git")
            message(FATAL_ERROR "${module} not found. Run git submodule update --init --recursive")
        endif()
    endforeach()
endmacro()

# Make sure all submodules are initialized, if we expect any.
if(EXISTS "${CMAKE_SOURCE_DIR}/.gitmodules")
    check_submodules()
endif()

# fmt
add_subdirectory(fmt)

# GoogleTest
if(VTILS_OPT_BUILD_TESTS)
    FetchContent_Declare(googletest
            GIT_REPOSITORY "https://github.com/google/googletest.git"
            GIT_TAG "main"
            )
    FetchContent_MakeAvailable(googletest)

    FetchContent_GetProperties(googletest)
    if(NOT googletest_POPULATED)
        FetchContent_Populate(googletest)
        add_subdirectory(${googletest_SOURCE_DIR} ${googletest_BINARY_DIR})
    endif()

    # Make sure that IDEs play nicely with the targets.
    set_target_properties(gtest gtest_main gmock gmock_main PROPERTIES FOLDER "third-party")

    # Keeps the cache cleaner.
    mark_as_advanced(
            BUILD_GMOCK BUILD_GTEST BUILD_SHARED_LIBS
            gmock_build_tests gtest_build_samples gtest_build_tests
            gtest_disable_pthreads gtest_force_shared_crt gtest_hide_internal_symbols
    )
endif()
