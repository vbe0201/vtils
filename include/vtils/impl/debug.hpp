/**
 * @file debug.hpp
 * @brief Portable breakpoints for debugging.
 * @copyright Valentin B.
 */
#pragma once

#include "vtils/macros/arch.hpp"
#include "vtils/macros/attr.hpp"
#include "vtils/macros/compiler.hpp"

#if defined(V_COMPILER_MSVC)
    #define V_DEBUG_BREAK_IMPL() __debugbreak()
#elif defined(V_ARCH_X64) || defined(V_ARCH_X86)
    // It is actually important to use int3 instead of int $3 here.
    // While GAS optimizes both to desired CC, NASM produces CD 03.
    #define V_DEBUG_BREAK_IMPL() __asm__ __volatile__("int3")
#elif defined(__thumb__)
    // See arm-linux-tdep.c in GDB source, eabi_linux_thumb_le_breakpoint.
    #define V_DEBUG_BREAK_IMPL() __asm__ __volatile__(".inst 0xd4200000")
#elif defined(V_ARCH_AARCH64)
    // See aarch64-tdep.c in GDB source, aarch64_default_breakpoint.
    #define V_DEBUG_BREAK_IMPL() __asm__ __volatile__(".inst 0xd4200000")
#elif defined(V_ARCH_ARM)
    // See arm-linux-tdep.c in GDB source, eabi_linux_arm_le_breakpoint.
    #define V_DEBUG_BREAK_IMPL() __asm__ __volatile__(".inst 0xe7f001f0")
#elif defined(V_ARCH_RISCV)
    // See riscv-tdep.c in GDB source, riscv_sw_breakpoint_from_kind.
    #define V_DEBUG_BREAK_IMPL() __asm__ __volatile__(".4byte 0x00100073")
#else
    #include <signal.h>
    #if defined(SIGTRAP)
        #define V_DEBUG_BREAK_IMPL() raise(SIGTRAP)
    #else
        #define V_DEBUG_BREAK_IMPL() raise(SIGABRT)
    #endif
#endif

namespace vtils::impl {

    /// Triggers a breakpoint trap for debugging when this code is hit.
    ALWAYS_INLINE void DebugBreak() {
        V_DEBUG_BREAK_IMPL();
    }

}

#undef V_DEBUG_BREAK_IMPL
