/**
 * @file attr.hpp
 * @brief C++ attributes as portable macros.
 * @copyright Valentin B.
 */
#pragma once

#include "vtils/macros/compiler.hpp"

/// Strong hint to the compiler to inline a function.
#ifndef ALWAYS_INLINE
    #ifdef V_COMPILER_MSVC
        #define ALWAYS_INLINE [[msvc::always_inline]]
    #else
        #define ALWAYS_INLINE [[gnu::always_inline]] inline
    #endif
#endif

/// Strong hint to the compiler to inline a lambda body at call-site.
#ifndef ALWAYS_INLINE_LAMBDA
    #ifdef V_COMPILER_MSVC
        #define ALWAYS_INLINE_LAMBDA [[msvc::forceinline]]
    #else
        #define ALWAYS_INLINE_LAMBDA __attribute__((always_inline))
    #endif
#endif

/// Indicates that a function should never be inlined.
#ifndef NOINLINE
    #define NOINLINE [[noreturn]]
#endif

/// Hints to the compiler that this function is unlikely to be executed.
#ifndef COLD
    #ifdef V_COMPILER_MSVC
        #define COLD
    #else
        #define COLD [[gnu::cold]]
    #endif
#endif

/// Hints to the compiler that this function never returns.
#ifndef NORETURN
    #define NORETURN [[noreturn]]
#endif

/// Hint to branch prediction that a path is likely to be taken.
#ifndef LIKELY
    #define LIKELY [[likely]]
#endif

/// Hint to branch prediction that a path is likely to not be taken.
#ifndef UNLIKELY
    #define UNLIKELY [[unlikely]]
#endif

/// Hints to the compiler that a certain expression holds true for optimization.
///
/// This code may or may not evaluate expr, which could be significant if the
/// evaluation has side effects or take a long time. Behavior may also differ
/// if the evaluation causes an exception to be thrown or a function never returns.
///
/// Special care must therefore be applied when using it without C++23 support
/// for the <a href="https://en.cppreference.com/w/cpp/language/attributes/assume">[[assume(expr)]]</a>
/// attribute.
#ifndef ASSUME
    #if __has_cpp_attribute(assume)
        #define ASSUME(expr) [[assume(expr)]]
    #else
        #if defined(V_COMPILER_MSVC)
            #define ASSUME(expr) __assume(expr)
        #elif defined(V_COMPILER_CLANG)
            #define ASSUME(expr) __builtin_assume(expr)
        #elif __GNUC__ >= 13
            // GCC 13 blesses us with gnu::assume which is available
            // to C++20 code on a recent enough compiler.
            #define ASSUME(expr) [[gnu::assume(expr)]]
        #else
            #define ASSUME(expr) do { if (!static_cast<bool>(expr)) { __builtin_unreachable(); } } while (false)
        #endif
    #endif
#endif

/// Raises a warning when the return value of a function is discarded.
#ifndef NODISCARD
    #define NODISCARD [[nodiscard]]
#endif
