/**
 * @file compiler.hpp
 * @brief Macros for detecting the C++ compiler in use.
 * @copyright Valentin B.
 */
#pragma once

#if defined(__clang__)
    #define V_COMPILER clang
    #define V_COMPILER_CLANG 1
#elif defined(__GNUC__)
    #define V_COMPILER gcc
    #define V_COMPILER_GCC 1
#elif defined(_MSC_VER)
    #define V_COMPILER msvc
    #define V_COMPILER_MSVC 1
#else
    #error "Compiler in use is currently not supported by vtils!"
#endif
