/**
 * @file memory_mapped.os.windows.hpp
 * @brief Memory-mapped files for Windows.
 * @copyright Valentin B.
 */
#pragma once

#include <cstdint>
#include <cstdio>

#include <io.h>
#include <handleapi.h>
#include <memoryapi.h>
#include <windef.h>

#include "vtils/macros/attr.hpp"

namespace vtils::impl {

    constexpr inline DWORD MmapResultSuccess      = 0;
    constexpr inline DWORD MmapResultFileTooLarge = 223;

    class MemoryMapped final {
    private:
        // Span information for the mapped memory region.
        void *m_ptr = nullptr;
        std::size_t m_len = 0;

        HANDLE m_handle = INVALID_HANDLE_VALUE;

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

        DWORD Map(HANDLE handle, DWORD protect, DWORD access, std::size_t offset, std::size_t len) noexcept;

        void Unmap() noexcept;

        DWORD Flush(std::size_t offset, std::size_t len) noexcept;

        DWORD FlushAsync(std::size_t offset, std::size_t len) noexcept;
    };

    DWORD GetFileSize(HANDLE handle, std::uint64_t *size);

    ALWAYS_INLINE HANDLE GetFileHandle(FILE *file) {
        return reinterpret_cast<HANDLE>(_get_osfhandle(_fileno(file)));
    }

}
