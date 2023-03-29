/**
 * @file features.hpp
 * @brief Target CPU feature detection at compile-time.
 * @copyright Valentin B.
 */
#pragma once

#include "vtils/macros/arch.hpp"
#include "vtils/macros/compiler.hpp"

#if defined(V_ARCH_X64) || defined(V_ARCH_X86)
    #if defined(V_COMPILER_MSVC)
        // `_M_IX86_FP` is defined only on 32-bit, 64-bit has SSE2 always.
        // https://docs.microsoft.com/en-us/cpp/preprocessor/predefined-macros
        #if (defined(_M_IX86_FP) && _M_IX86_FP == 2) || defined(V_ARCH_X64)
            #define V_TARGET_FEATURE_SSE2 1
        #endif

        // There seems to be no way to directly detect SSE3 or newer.
        // Best we can do is base it off AVX support.
        #ifdef __AVX__
            #define V_TARGET_FEATURE_SSE3 1
            #define V_TARGET_FEATURE_SSSE3 1
            #define V_TARGET_FEATURE_SSE4_1 1
            #define V_TARGET_FEATURE_SSE4_2 1
        #endif
    #else
        // GCC and Clang both define these:
        // https://stackoverflow.com/a/28939692
        #ifdef __SSE2__
            #define V_TARGET_FEATURE_SSE2 1
        #endif
        #ifdef __SSE3__
            #define V_TARGET_FEATURE_SSE3 1
        #endif
        #ifdef __SSSE3__
            #define V_TARGET_FEATURE_SSSE3 1
        #endif
        #ifdef __SSE4_1__
            #define V_TARGET_FEATURE_SSE4_1 1
        #endif
        #ifdef __SSE4_2__
            #define V_TARGET_FEATURE_SSE4_2 1
        #endif
    #endif

    // These macros are common to all supported compilers.
    #ifdef __AVX__
        #define V_TARGET_FEATURE_AVX 1
    #endif
    #ifdef __AVX2__
        #define V_TARGET_FEATURE_AVX2 1
    #endif
    #ifdef __AVX512F__
        #define V_TARGET_FEATURE_AVX512F 1
    #endif
#elif defined(V_ARCH_AARCH64) || defined(V_ARCH_ARM)
    #ifdef __ARM_NEON
        #define V_TARGET_FEATURE_NEON 1

        // NEON FMA is available only if __ARM_FEATURE_FMA is defined and some bits
        // of __ARM_NEON_FP as well. On AArch64, NEON is implicitly supported and
        // __ARM_NEON_FP might not be defined, so check for AArch64 as well.
        #if defined(__ARM_FEATURE_FMA) && (__ARM_NEON_FP || defined(V_ARCH_AARCH64))
            #define V_TARGET_FEATURE_NEON_FMA 1
        #endif

        // The FP16 instructions in GCC and Clang are guarded by this, should be fine.
        // https://gcc.gnu.org/legacy-ml/gcc-patches/2016-06/msg00460.html
        #ifdef __ARM_FEATURE_FP16_VECTOR_ARITHMETIC
            #define V_TARGET_FEATURE_NEON_FP16 1
        #endif
    #endif
#elif defined(V_ARCH_WASM)
    // Undocumented, checked with `echo | em++ -x c++ -dM -E - -msimd128`.
    // Finalized SIMD variant was added in Clang 13, emsdk 2.0.18 started shipping it:
    // https://github.com/llvm/llvm-project/commit/502f54049d17f5a107f833596fb2c31297a99773
    #if defined(__wasm_simd128__) && __clang_major__ >= 13 && V_ARCH_EMSCRIPTEN_VERSION >= 20018
        #define V_TARGET_FEATURE_SIMD128 1
    #endif
#endif
