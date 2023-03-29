/**
 * @file target.hpp
 * @brief Portable target platform detection macros.
 * @copyright Valentin B.
 */
#pragma once

#if defined(_WIN64)
    #define V_PLATFORM win64
    #define V_PLATFORM_WINDOWS 1
    #define V_PLATFORM_WIN64 1
#elif defined(_WIN32)
    #define V_PLATFORM win32
    #define V_PLATFORM_WINDOWS 1
    #define V_PLATFORM_WIN32 1
#elif defined(__linux__)
    #define V_PLATFORM linux
    #define V_PLATFORM_LINUX 1
#elif defined(__APPLE__) && defined(__MACH__)
    #include <TargetConditionals.h>

    #define V_PLATFORM_APPLE 1
    #if TARGET_IPHONE_SIMULATOR == 1
        #define V_PLATFORM iphone_simulator
        #define V_PLATFORM_IPHONE_SIM 1
    #elif TARGET_OS_IPHONE == 1
        #define V_PLATFORM iphone
        #define V_PLATFORM_IPHONE 1
    #elif TARGET_OS_MAC == 1
        #define V_PLATFORM macos
        #define V_PLATFORM_MACOS 1
    #endif
#elif defined(__DragonFly__)
    #define V_PLATFORM dragonfly
    #define V_PLATFORM_DRAGONFLY 1
#elif defined(__FreeBSD__)
    #define V_PLATFORM freebsd
    #define V_PLATFORM_FREEBSD 1
#elif defined(__NetBSD__)
    #define V_PLATFORM netbsd
    #define V_PLATFORM_NETBSD 1
#elif defined(__OpenBSD__)
    #define V_PLATFORM openbsd
    #define V_PLATFORM_OPENBSD 1
#elif defined(__sun) && defined(__SVR4)
    #define V_PLATFORM solaris
    #define V_PLATFORM_SOLARIS 1
#endif
