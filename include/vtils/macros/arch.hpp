/**
 * @file arch.hpp
 * @brief Target architecture detection for portable platform code.
 * @copyright Valentin B.
 */
#pragma once

#if defined(__x86_64) || defined(__x86_64__) || defined(__amd64) || defined(_M_X64)
    #define V_ARCH x86_64
    #define V_ARCH_X64 1
#elif defined(__ARM64__) || defined(__aarch64__) || defined(_M_ARM64)
    #define V_ARCH aarch64
    #define V_ARCH_AARCH64 1
#elif defined(__arm__) || defined(__TARGET_ARCH_ARM) || defined(_M_ARM)
    #define V_ARCH arm
    #define V_ARCH_ARM 1
#elif defined(__i386) || defined(__i386__) || defined(_M_IX86)
    #define V_ARCH x86
    #define V_ARCH_X86 1
#elif defined(__riscv)
    #define V_ARCH riscv
    #define V_ARCH_RISCV 1
#elif defined(__EMSCRIPTEN__)
    #define V_ARCH wasm
    #define V_ARCH_WASM 1

    #define V_ARCH_EMSCRIPTEN_VERSION (__EMSCRIPTEN_major__*10000 + __EMSCRIPTEN_minor__*100 + __EMSCRIPTEN_tiny__)
#else
    #define V_ARCH generic
    #define V_ARCH_GENERIC 1
#endif
