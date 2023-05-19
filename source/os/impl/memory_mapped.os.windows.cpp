#include "vtils/os/impl/memory_mapped.os.windows.hpp"

#include <windows.h>

#include "vtils/alignment.hpp"
#include "vtils/scope_guard.hpp"
#include "vtils/macros/misc.hpp"

namespace vtils::impl {

    namespace {

        const std::size_t AllocationGranularity = [] {
            SYSTEM_INFO info;
            ::GetSystemInfo(std::addressof(info));
            return info.dwAllocationGranularity;
        }();

    }

    MemoryMapped::~MemoryMapped() {
        this->Unmap();
    }

    MemoryMapped::MemoryMapped(MemoryMapped &&rhs) noexcept
        : m_ptr(rhs.m_ptr), m_len(rhs.m_len), m_handle(rhs.m_handle)
    {
        // Reset `rhs` back into default state.
        rhs.m_ptr    = nullptr;
        rhs.m_len    = 0;
        rhs.m_handle = INVALID_HANDLE_VALUE;
    }

    MemoryMapped &MemoryMapped::operator=(MemoryMapped &&rhs) noexcept {
        // Remove an existing mapping from the target.
        this->Unmap();

        // Copy mapping details from `rhs`.
        m_ptr    = rhs.m_ptr;
        m_len    = rhs.m_len;
        m_handle = rhs.m_handle;

        // Reset `rhs` back into default state.
        rhs.m_ptr    = nullptr;
        rhs.m_len    = 0;
        rhs.m_handle = INVALID_HANDLE_VALUE;

        return *this;
    }

    DWORD MemoryMapped::Map(HANDLE handle, DWORD protect, DWORD access, std::size_t offset, std::size_t len) noexcept {
        // Compute offset and length of allocation with respect to granularity.
        const std::size_t aligned_offset = AlignDown(offset, AllocationGranularity);
        const std::size_t alignment      = offset - aligned_offset;
        const std::size_t aligned_len    = len + alignment;

        // Explicitly check for length 0 as we're not allowed to allocate that.
        V_DEBUG_ASSERT(len != 0);

        // Create the file mapping from the handle.
        HANDLE mapping = ::CreateFileMappingW(handle, nullptr, protect, 0, 0, nullptr);
        if (mapping == nullptr) {
            return ::GetLastError();
        }
        V_ON_SCOPE_EXIT { ::CloseHandle(mapping); };

        // Map the file view into memory.
        auto *ptr = ::MapViewOfFile(mapping, access, aligned_offset >> BITSIZEOF(DWORD), aligned_offset, aligned_len);
        if (ptr == nullptr) {
            return ::GetLastError();
        }

        // Duplicate the supplied handle so that the object can own it.
        HANDLE duplicated;
        HANDLE current_process = ::GetCurrentProcess();
        if (::DuplicateHandle(current_process, handle, current_process, std::addressof(duplicated), 0, 0, DUPLICATE_SAME_ACCESS) == FALSE) {
            ::UnmapViewOfFile(ptr);
            return ::GetLastError();
        }

        V_DEBUG_ASSERT(duplicated != nullptr && duplicated != INVALID_HANDLE_VALUE);

        // Store the newly created state in the object.
        m_ptr    = static_cast<std::uint8_t *>(ptr) + alignment;
        m_len    = len;
        m_handle = duplicated;

        return MmapResultSuccess;
    }

    void MemoryMapped::Unmap() noexcept {
        // If we don't maintain a mapping, we have nothing to do.
        if (!this->IsMapped()) {
            return;
        }

        // Compute pointer alignment to revert applied offsets.
        const std::size_t alignment = reinterpret_cast<std::uintptr_t>(m_ptr) % AllocationGranularity;

        // Unmap the file view and close the stored handle.
        ::UnmapViewOfFile(static_cast<std::uint8_t *>(m_ptr) - alignment);
        ::CloseHandle(m_handle);
    }

    DWORD MemoryMapped::Flush(std::size_t offset, std::size_t len) noexcept {
        // Attempt to flush the memory region asynchronously.
        auto res = this->FlushAsync(offset, len);

        // If we have a mapping and no error occurred, flush the file buffers.
        if ((res == MmapResultSuccess) && this->IsMapped()) {
            if (::FlushFileBuffers(m_handle) == FALSE) {
                res = ::GetLastError();
            }
        }

        return res;
    }

    DWORD MemoryMapped::FlushAsync(std::size_t offset, std::size_t len) noexcept {
        // If we don't maintain a mapping, we have nothing to do.
        if (!this->IsMapped()) {
            return MmapResultSuccess;
        }

        // Flush the file view of the mapping.
        if (::FlushViewOfFile(static_cast<std::uint8_t *>(m_ptr) + offset, len) == FALSE) {
            return ::GetLastError();
        }

        return MmapResultSuccess;
    }

    DWORD GetFileSize(HANDLE handle, std::uint64_t *size) {
        // Try to query file size information.
        BY_HANDLE_FILE_INFORMATION info;
        if (::GetFileInformationByHandle(handle, std::addressof(info)) == FALSE) {
            return ::GetLastError();
        }

        // On success, write to output pointer.
        *size = (static_cast<std::uint64_t>(info.nFileSizeHigh) << BITSIZEOF(DWORD)) | static_cast<std::uint64_t>(info.nFileSizeLow);
        return MmapResultSuccess;
    }

}
