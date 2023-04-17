/**
 * @file misc.hpp
 * @brief Miscellaneous useful macros.
 * @copyright Valentin B.
 */
#pragma once

#include <limits.h>

#include "vtils/macros/attr.hpp"

/// @exclude
#define V_STRINGIFY_IMPL(s) #s
/// Converts an arbitrary expression to a string literal.
#define V_STRINGIFY(s) V_STRINGIFY_IMPL(s)

/// @exclude
#define V_CONCAT_IMPL(s1, s2) s1##s2
/// Textually concatenates two given tokens.
#define V_CONCAT(s1, s2) V_CONCAT_IMPL(s1, s2)

/// Creates an anonymous variable with a unique name.
#ifdef __COUNTER__
    #define V_ANON_VAR(x) V_CONCAT(x, __COUNTER__)
#else
    #define V_ANON_VAR(x) V_CONCAT(x, __LINE__)
#endif

/// Gets the size of a type in bits.
#ifndef BITSIZEOF
    #define BITSIZEOF(T) (sizeof(T) * CHAR_BIT)
#endif

/// Discards unused variables without side effects.
#define V_UNUSED(...) ::vtils::impl::UnusedImpl(__VA_ARGS__)

namespace vtils::impl {

    /// @exclude
    template <typename... Args>
    ALWAYS_INLINE constexpr void UnusedImpl(Args &&...args) {
        (static_cast<void>(args), ...);
    }

}
