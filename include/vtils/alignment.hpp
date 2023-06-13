/**
 * @file alignment.hpp
 * @brief Memory alignment to powers of two.
 * @copyright Valentin B.
 */
#pragma once

#include <concepts>
#include <cstdint>
#include <type_traits>

#include "vtils/assert.hpp"
#include "vtils/macros/attr.hpp"

namespace vtils {

    namespace impl {

        // TODO: Move this elsewhere?
        template <std::integral I>
        ALWAYS_INLINE constexpr bool IsPowerOfTwo(I value) {
            return (value > 0) && ((value & (value - 1)) == 0);
        }

    }

    /// Aligns `value` up to the next multiple of `align`.
    ///
    /// `align` must be a power of two.
    template <typename T>
    ALWAYS_INLINE constexpr T AlignUp(T value, std::size_t align) {
        using U = std::make_unsigned_t<T>;

        V_DEBUG_ASSERT(impl::IsPowerOfTwo(align));

        const U mask = static_cast<U>(align - 1);
        return static_cast<T>((value + mask) & ~mask);
    }

    /// Aligns `value` down to the next multiple of `align`.
    ///
    /// `align` must be a power of two.
    template <typename T>
    ALWAYS_INLINE constexpr T AlignDown(T value, std::size_t align) {
        using U = std::make_unsigned_t<T>;

        V_DEBUG_ASSERT(impl::IsPowerOfTwo(align));

        const U mask = static_cast<U>(align - 1);
        return static_cast<T>(value & ~mask);
    }

    /// Checks if `value` is aligned to boundaries of `align`.
    ///
    /// `align` must be a power of two.
    template <typename T>
    ALWAYS_INLINE constexpr bool IsAligned(T value, std::size_t align) {
        using U = std::make_unsigned_t<T>;

        V_DEBUG_ASSERT(impl::IsPowerOfTwo(align));

        const U mask = static_cast<U>(align - 1);
        return (value & mask) == 0;
    }

    template <>
    ALWAYS_INLINE void *AlignUp<void *>(void *value, std::size_t align) {
        return reinterpret_cast<void *>(AlignUp(reinterpret_cast<std::uintptr_t>(value), align));
    }

    template <>
    ALWAYS_INLINE const void *AlignUp<const void *>(const void *value, std::size_t align) {
        return reinterpret_cast<const void *>(AlignUp(reinterpret_cast<std::uintptr_t>(value), align));
    }

    template <>
    ALWAYS_INLINE void *AlignDown<void *>(void *value, std::size_t align) {
        return reinterpret_cast<void *>(AlignDown(reinterpret_cast<std::uintptr_t>(value), align));
    }

    template <>
    ALWAYS_INLINE const void *AlignDown<const void *>(const void *value, std::size_t align) {
        return reinterpret_cast<const void *>(AlignDown(reinterpret_cast<std::uintptr_t>(value), align));
    }

    template <>
    ALWAYS_INLINE bool IsAligned<void *>(void *value, std::size_t align) {
        return IsAligned(reinterpret_cast<std::uintptr_t>(value), align);
    }

    template <>
    ALWAYS_INLINE bool IsAligned<const void *>(const void *value, std::size_t align) {
        return IsAligned(reinterpret_cast<std::uintptr_t>(value), align);
    }

}
