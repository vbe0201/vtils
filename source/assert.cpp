#include "vtils/assert.hpp"

#include "vtils/impl/debug.hpp"

namespace vtils::impl {

    // TODO: C++23 `std::backtrace` support once available?

    COLD NOINLINE NORETURN void AssertionFailed(const std::source_location &source, const char *expr) {
        fmt::print(stderr, "Assertion failed!\n");
        fmt::print(stderr, "    Expression: {}\n", expr);
        fmt::print(stderr, "    Function:   {}\n", source.function_name());
        fmt::print(stderr, "    Location:   {}:{}\n", source.file_name(), source.line());

        // The `DebugBreak` makes debugging easier when a debugger is attached. However,
        // that alone doesn't fulfill the NORETURN guarantee we maintain so we use a loop.
        while (true) {
            DebugBreak();
        }
    }

    COLD NOINLINE NORETURN void MonomorphizedAssertionFailed(const std::source_location &source, const char *expr, std::string_view fmt, fmt::format_args args) {
        fmt::print(stderr, "Assertion failed: ");
        fmt::vprint(stderr, fmt, args);
        fmt::print(stderr, "!\n");
        fmt::print(stderr, "    Expression: {}\n", expr);
        fmt::print(stderr, "    Function:   {}\n", source.function_name());
        fmt::print(stderr, "    Location:   {}:{}\n", source.file_name(), source.line());

        // The `DebugBreak` makes debugging easier when a debugger is attached. However,
        // that alone doesn't fulfill the NORETURN guarantee we maintain so we use a loop.
        while (true) {
            DebugBreak();
        }
    }

}
