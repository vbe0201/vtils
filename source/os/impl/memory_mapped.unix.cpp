#include "vtils/os/impl/memory_mapped.unix.hpp"

#include <unistd.h>
#include <sys/stat.h>

#include "vtils/alignment.hpp"
#include "vtils/assert.hpp"
#include "vtils/macros/arch.hpp"
#include "vtils/macros/platform.hpp"

namespace vtils::impl {

    namespace {

        const std::size_t AllocationGranularity = sysconf(_SC_PAGE_SIZE);

    }

    MemoryMapped::~MemoryMapped() {
        this->Unmap();
    }

    MemoryMapped::MemoryMapped(MemoryMapped &&rhs) noexcept : m_ptr(rhs.m_ptr), m_len(rhs.m_len) {
        // Reset `rhs` back into default state.
        rhs.m_ptr    = nullptr;
        rhs.m_len    = 0;
    }

    MemoryMapped &MemoryMapped::operator=(MemoryMapped &&rhs) noexcept {
        // Remove an existing mapping from the target.
        this->Unmap();

        // Copy mapping details from `rhs`.
        m_ptr    = rhs.m_ptr;
        m_len    = rhs.m_len;

        // Reset `rhs` back into default state.
        rhs.m_ptr    = nullptr;
        rhs.m_len    = 0;

        return *this;
    }

    int MemoryMapped::Map(int fd, int protect, int flags, std::size_t offset, std::size_t len) noexcept {
        // Compute offset and length of allocation with respect to granularity.
        const std::size_t aligned_offset = AlignDown(offset, AllocationGranularity);
        const std::size_t alignment      = offset - aligned_offset;
        const std::size_t aligned_len    = len + alignment;

        // Explicitly check for len 0 as we're not allowed to allocate that.
        V_DEBUG_ASSERT(aligned_len != 0);

        // Map the file view into memory.
        auto *ptr = mmap(nullptr, aligned_len, protect, flags, fd, static_cast<off_t>(aligned_offset));
        if (ptr == MAP_FAILED) {
            return errno;
        }

        // Commit the newly created state onto this object.
        m_ptr = static_cast<std::uint8_t *>(ptr) + alignment;
        m_len = len;

        return MmapResultSuccess;
    }

    void MemoryMapped::Unmap() noexcept {
        // If we don't maintain a mapping, we have nothing to do.
        if (this->IsMapped()) {
            return;
        }

        // Compute the pointer alignment to revert applied offsets from mapping.
        const std::size_t alignment = reinterpret_cast<std::uintptr_t>(m_ptr) % AllocationGranularity;

        // Unmap the file view.
        munmap(static_cast<std::uint8_t *>(m_ptr) - alignment, m_len + alignment);
    }

    int MemoryMapped::Flush(std::size_t offset, std::size_t len) noexcept {
        // If we don't maintain a mapping, we have nothing t odo.
        if (this->IsMapped()) {
            return MmapResultSuccess;
        }

        auto *ptr = static_cast<std::uint8_t *>(m_ptr);

        // Compute offset and length of allocation with respect to granularity.
        const std::size_t alignment        = reinterpret_cast<std::uintptr_t>(ptr + offset) % AllocationGranularity;
        const std::size_t unaligned_offset = offset - alignment;
        const std::size_t unaligned_len    = len + alignment;

        // Attempt to flush the memory region.
        if (msync(ptr + unaligned_offset, unaligned_len, MS_SYNC) == 0) {
            return MmapResultSuccess;
        } else {
            return errno;
        }
    }

    int MemoryMapped::FlushAsync(std::size_t offset, std::size_t len) noexcept {
        // If we don't maintain a mapping, we have nothing t odo.
        if (this->IsMapped()) {
            return MmapResultSuccess;
        }

        auto *ptr = static_cast<std::uint8_t *>(m_ptr);

        // Compute offset and length of allocation with respect to granularity.
        const std::size_t alignment        = reinterpret_cast<std::uintptr_t>(ptr + offset) % AllocationGranularity;
        const std::size_t unaligned_offset = offset - alignment;
        const std::size_t unaligned_len    = len + alignment;

        // Attempt to flush the memory region.
        if (msync(ptr + unaligned_offset, unaligned_len, MS_ASYNC) == 0) {
            return MmapResultSuccess;
        } else {
            return errno;
        }
    }

    int GetFileSize(int fd, std::uint64_t *size) {
    #if defined(V_PLATFORM_LINUX) || defined(V_ARCH_WASM)
        #define V_TEMP_FSTAT fstat64
        #define V_TEMP_STAT  struct stat64
    #else
        #define V_TEMP_FSTAT fstat
        #define V_TEMP_STAT  struct stat
    #endif

        // Try to query file size information by file descriptor.
        V_TEMP_STAT sb{};
        if (V_TEMP_FSTAT(fd, std::addressof(sb)) == 0) {
            *size = sb.st_size;
            return MmapResultSuccess;
        } else {
            return errno;
        }

        #undef V_TEMP_FSTAT
        #undef V_TEMP_STAT
    }

}
