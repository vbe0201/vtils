/**
 * @file memory_mapped.unix.cpp
 * @brief Memory-mapped files for UNIX platforms.
 * @copyright Valentin B.
 */
#pragma once

#include <cerrno>
#include <cstdint>

#include <errno.h>
#include <stdio.h>

#include "vtils/macros/attr.hpp"

namespace vtils::impl {

    constexpr inline int MmapResultSuccess      = 0;
    constexpr inline int MmapResultFileTooLarge = E2BIG;

    class MemoryMapped final {
    private:
        // Span information for the mapped memory region.
        void *m_ptr = nullptr;
        std::size_t m_len = 0;

    public:
        ALWAYS_INLINE MemoryMapped() = default;

        ~MemoryMapped();

        MemoryMapped(MemoryMapped &&rhs) noexcept;

        MemoryMapped &operator=(MemoryMapped &&rhs) noexcept;

        ALWAYS_INLINE bool IsMapped() const {
            return m_ptr != nullptr;
        }

        ALWAYS_INLINE void *GetPtr() {
            return m_ptr;
        }

        ALWAYS_INLINE const void *GetPtr() const {
            return m_ptr;
        }

        ALWAYS_INLINE std::size_t GetLength() const {
            return m_len;
        }

        int Map(int fd, int protect, int flags, std::size_t offset, std::size_t len) noexcept;

        void Unmap() noexcept;

        int Flush(std::size_t offset, std::size_t len) noexcept;

        int FlushAsync(std::size_t offset, std::size_t len) noexcept;
    };

    int GetFileSize(int fd, std::uint64_t *size);

    ALWAYS_INLINE int GetFileHandle(FILE *file) {
        return fileno(file);
    }

}
