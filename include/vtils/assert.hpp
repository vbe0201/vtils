/**
 * @file assert.hpp
 * @brief Runtime assertions for ensuring code robustness.
 * @copyright Valentin B.
 */
#pragma once

#include <source_location>
#include <string_view>
#include <type_traits>

#include <fmt/core.h>

#include "vtils/macros/attr.hpp"

/// Asserts a given condition and aborts execution when not met.
///
/// Optionally accepts a format string and arguments for further
/// context in stderr output.
#define V_ASSERT(expr, ...) V_ASSERT_IMPL(expr __VA_OPT__(,) __VA_ARGS__)

/// Asserts a given condition in debug builds and aborts execution
/// when not met.
///
/// In release builds, the asserted condition may be turned into a
/// @ref ASSUME hint to the compiler to optimize around it.
#if defined(V_ENABLE_DEBUG_ASSERTIONS)
    #define V_DEBUG_ASSERT(expr, ...) V_ASSERT(expr __VA_OPT__(,) __VA_ARGS__)
#else
    #define V_DEBUG_ASSERT(expr, ...) ASSUME(expr)
#endif

/// Unconditionally triggers an assertion failure at the current position.
///
/// Optionally accepts a format string and arguments for further context
/// in stderr output.
///
/// This is useful for debugging or handling error conditions that cannot
/// be recovered from.
#define V_ABORT(...) V_ASSERT_IMPL(false __VA_OPT__(,) __VA_ARGS__)

/// Indicates that a branch is never executed and therefore triggers an
/// assertion failure when hit.
#define V_UNREACHABLE() V_ASSERT_IMPL(false, "entered unreachable code")

/// Indicates that code is not implemented and therefore triggers an
/// assertion failure when hit.
///
/// Use this when code is not intended to be implemented at all. See
/// @ref V_TODO otherwise.
#define V_UNIMPLEMENTED() V_ASSERT_IMPL(false, "not implemented")

/// Indicates that code is not yet implemented, but is planned to be
/// in the future. Triggers an assertion failure when hit.
///
/// See @ref V_UNIMPLEMENTED if you're not actually planning to implement
/// this code at a later time.
#define V_TODO() V_ASSERT_IMPL(false, "not yet implemented")

#define V_ASSERT_IMPL(expr, ...)                                                              \
    [&, __v_assert_impl_source = std::source_location::current()]() ALWAYS_INLINE_LAMBDA {    \
        if (const bool __expr = static_cast<bool>(expr); !__expr) UNLIKELY {                  \
            V_CALL_ASSERT_FAIL_IMPL(__v_assert_impl_source, #expr __VA_OPT__(,) __VA_ARGS__); \
        }                                                                                     \
    }()

#define V_CALL_ASSERT_FAIL_IMPL(source, expr, ...)                                       \
    [&](const std::source_location &source) ALWAYS_INLINE_LAMBDA {                        \
        if (std::is_constant_evaluated()) {                                              \
            ::vtils::impl::ConstexprAssertionFailed("assertion failed at compile-time"); \
        } else {                                                                         \
            ::vtils::impl::AssertionFailed(source, expr __VA_OPT__(,) __VA_ARGS__);      \
        }                                                                                \
    }(source)

namespace vtils::impl {

    inline void ConstexprAssertionFailed(const char *) {}

    COLD NOINLINE NORETURN void AssertionFailed(const std::source_location &source, const char *expr);
    COLD NOINLINE NORETURN void MonomorphizedAssertionFailed(const std::source_location &source, const char *expr, std::string_view fmt, fmt::format_args args);

    template <typename... Args>
    COLD NORETURN void AssertionFailed(const std::source_location &source, const char *expr, std::string_view fmt, Args &&...args) {
        MonomorphizedAssertionFailed(source, expr, fmt, fmt::make_format_args(args...));
    }

}
