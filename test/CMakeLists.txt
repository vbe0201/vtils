include(GoogleTest)

function(vtils_test name)
    set(__VTILS_TEST run_${name}_tests)
    set(__VTILS_TEST_SRC test_${name}.cpp)

    add_executable(${__VTILS_TEST} ${CMAKE_CURRENT_SOURCE_DIR}/${__VTILS_TEST_SRC} ${ARGN})
    target_compile_features(${__VTILS_TEST} PRIVATE cxx_std_20)
    target_link_libraries(${__VTILS_TEST} ${PROJECT_NAME}::${PROJECT_NAME} gtest gmock gtest_main)
    set_target_properties(${__VTILS_TEST} PROPERTIES FOLDER "test")

    gtest_discover_tests(${__VTILS_TEST}
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")
endfunction()
