cmake_minimum_required(VERSION 3.23 FATAL_ERROR)

######################
## Project settings ##
######################

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

project(vtils
    VERSION "0.1.0"
    DESCRIPTION "Portable C++ utility library for personal use"
    HOMEPAGE_URL "https://github.com/vbe0201/vtils"
    LANGUAGES CXX
    )

# Determine whether vtils is the top-level CMake project.
# Certain build options will be configured based on that.
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    set(VTILS_TOP_LEVEL_PROJECT ON)
else()
    set(VTILS_TOP_LEVEL_PROJECT OFF)
endif()

# Customizable build options.
option(VTILS_OPT_BUILD_DOCS "Build project documentation" ${VTILS_TOP_LEVEL_PROJECT})
option(VTILS_OPT_BUILD_TESTS "Build and perform vtils tests" ${VTILS_TOP_LEVEL_PROJECT})
option(VTILS_OPT_INSTALL "Generate and install vtils target" ${VTILS_TOP_LEVEL_PROJECT})

# Enforce the C++ standard when this is the top-level project.
if(VTILS_TOP_LEVEL_PROJECT)
    set(CMAKE_CXX_STANDARD 23)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    set(CMAKE_CXX_EXTENSIONS OFF)
endif()

#####################
## Target settings ##
#####################

add_library(${PROJECT_NAME})
add_library(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

target_sources(${PROJECT_NAME}
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include/vtils/impl/debug.hpp

    PUBLIC FILE_SET HEADERS TYPE HEADERS FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/include/vtils/macros/arch.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include/vtils/macros/attr.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include/vtils/macros/compiler.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include/vtils/macros/features.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include/vtils/macros/misc.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include/vtils/macros/platform.hpp

        ${CMAKE_CURRENT_SOURCE_DIR}/include/vtils/os/condvar.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include/vtils/os/mutex.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include/vtils/os/read_write_lock.hpp

        ${CMAKE_CURRENT_SOURCE_DIR}/include/vtils/assert.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include/vtils/macros.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include/vtils/scope_guard.hpp

    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/source/assert.cpp
    )

if (WIN32)
    # These source files are only included for Windows builds.
    target_sources(${PROJECT_NAME}
        PUBLIC
            ${CMAKE_CURRENT_SOURCE_DIR}/include/vtils/os/impl/condvar.os.windows.hpp
            ${CMAKE_CURRENT_SOURCE_DIR}/include/vtils/os/impl/mutex.os.windows.hpp
            ${CMAKE_CURRENT_SOURCE_DIR}/include/vtils/os/impl/read_write_lock.os.windows.hpp
        )
endif()

target_include_directories(${PROJECT_NAME} PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
    )
target_link_libraries(${PROJECT_NAME} PUBLIC fmt::fmt)

if(VTILS_OPT_INSTALL)
    # TODO
endif()

####################
## Subdirectories ##
####################

add_subdirectory(third-party)

if(VTILS_OPT_BUILD_DOCS)
    add_subdirectory(docs)
endif()

if(VTILS_OPT_BUILD_TESTS)
    enable_testing()
    add_subdirectory(test)
endif()
