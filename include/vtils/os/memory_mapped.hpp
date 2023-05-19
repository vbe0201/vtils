/**
 * @file memory_mapped.hpp
 * @brief Portable memory-mapped files.
 * @copyright Valentin B.
 */
#pragma once

#include <system_error>

#include "vtils/macros/misc.hpp"
#include "vtils/macros/platform.hpp"

#if defined(V_PLATFORM_WINDOWS)
    #include "vtils/os/impl/memory_mapped.os.windows.hpp"
#else
    #include "vtils/os/impl/memory_mapped.unix.hpp"
#endif

namespace vtils {

    /// Access permissions for the mapped file.
    enum class AccessMode {
        /// The file is mapped as read-only.
        ReadOnly,
        /// The file is mapped readable and writable.
        ReadWrite,
    };

    /// Representation of a memory mapping backed by an underlying file.
    ///
    /// The given @ref AccessMode specifies how the file can be interacted
    /// with, and may not allow performing modifying operations on it.
    template <AccessMode Mode>
    class MemoryMapped {
    private:
        using HandleType = decltype(impl::GetFileHandle(std::declval<FILE *>()));
        using ErrorCode = decltype(impl::MmapResultSuccess);

        static constexpr ErrorCode Success = impl::MmapResultSuccess;

    private:
        impl::MemoryMapped m_impl;

    private:
        constexpr MemoryMapped() : m_impl() {}

        ALWAYS_INLINE static HandleType GetFileHandle(FILE *file) {
            return impl::GetFileHandle(file);
        }

        ALWAYS_INLINE ErrorCode MapImpl(HandleType handle, std::size_t offset, std::size_t len) {
        #if defined(V_PLATFORM_WINDOWS)
            DWORD access  = FILE_MAP_READ;
            DWORD protect = PAGE_READONLY;

            if constexpr (Mode == AccessMode::ReadWrite) {
                access |= FILE_MAP_WRITE;
                protect = PAGE_READWRITE;
            }
        #else
            int access  = MAP_SHARED;
            int protect = PROT_READ;

            if constexpr (Mode == AccessMode::ReadWrite) {
                protect |= PROT_WRITE;
            }
        #endif

            return m_impl.Map(handle, protect, access, offset, len);
        }

        ALWAYS_INLINE ErrorCode MapWithOffsetImpl(HandleType handle, std::size_t offset) {
            // Attempt to query the byte size for the given file.
            std::uint64_t size;
            if (auto res = impl::GetFileSize(handle, std::addressof(size)); res != impl::MmapResultSuccess) {
                return res;
            }

            // Check preconditions and compute the real length we need to map.
            V_ASSERT(size >= offset);
            const auto len = size - offset;

            // Make sure there's no memory map overflow on non-64-bit targets.
            if constexpr (BITSIZEOF(std::uintptr_t) != BITSIZEOF(std::uint64_t)) {
                if (len > std::numeric_limits<std::uint64_t>::max()) {
                    return impl::MmapResultFileTooLarge;
                }
            }

            // Map the file into memory.
            return this->MapImpl(handle, offset, len);
        }

    public:
        MemoryMapped(const MemoryMapped &) = delete;
        const MemoryMapped &operator=(const MemoryMapped &) = delete;

        MemoryMapped(MemoryMapped &&) = default;
        MemoryMapped &operator=(MemoryMapped &&) = default;

        /// Maps the full range of a file into memory.
        ///
        /// \throws std::system_error When the OS reported an error.
        ALWAYS_INLINE static MemoryMapped Map(FILE *file) {
            MemoryMapped mapped{};

            if (const auto res = mapped.MapWithOffsetImpl(MemoryMapped::GetFileHandle(file), 0); res != Success) {
                throw std::system_error(res, std::generic_category(), "failed to map file into memory");
            }

            return mapped;
        }

        /// Maps a file into memory, given a start offset for the mapping.
        ///
        /// \throws std::system_error When the OS reported an error.
        ALWAYS_INLINE static MemoryMapped MapWithOffset(FILE *file, std::size_t offset) {
            MemoryMapped mapped{};

            if (const auto res = mapped.MapWithOffsetImpl(MemoryMapped::GetFileHandle(file), offset); res != Success) {
                throw std::system_error(res, std::generic_category(), "failed to map file into memory");
            }

            return mapped;
        }

        /// Maps a file into memory, given a start offset and length for the mapping.
        ///
        /// \throws std::system_error When the OS reported an error.
        ALWAYS_INLINE static MemoryMapped MapWithOffsetAndLength(FILE *file, std::size_t offset, std::size_t len) {
            MemoryMapped mapped{};

            if (const auto res = mapped.MapImpl(MemoryMapped::GetFileHandle(file), offset, len); res != Success) {
                throw std::system_error(res, std::generic_category(), "failed to map file into memory");
            }

            return mapped;
        }

        /// Gets a mutable pointer to the mapped memory region.
        ALWAYS_INLINE void *GetPtr() {
            static_assert(Mode == AccessMode::ReadWrite, "cannot modify read-only mapping");
            return m_impl.GetPtr();
        }

        /// Gets a const pointer to the mapped memory region.
        ALWAYS_INLINE const void *GetPtr() const {
            return m_impl.GetPtr();
        }

        /// Gets the length in bytes of the mapped memory region.
        ALWAYS_INLINE std::size_t GetLength() const {
            return m_impl.GetLength();
        }

        /// Flushes outstanding memory modifications to disk.
        ///
        /// When the method does not error, it is guaranteed that all outstanding
        /// changes were durably stored to the underlying file.
        ///
        /// \throws std::system_error When the OS reported an error.
        ALWAYS_INLINE void Flush() {
            static_assert(Mode == AccessMode::ReadWrite, "can only flush mutable file buffers");

            if (const auto res = m_impl.Flush(0, this->GetLength()); res != Success) {
                throw std::system_error(res, std::generic_category(), "failed to flush memory mapping to disk");
            }
        }

        /// Asynchronously flushes outstanding memory modifications to disk.
        ///
        ///  This method initiates flushing the modified pages to durable storage, but
        /// will not wait for the operation to complete before returning.
        ///
        /// \throws std::system_error When the OS reported an error.
        ALWAYS_INLINE void FlushAsync() {
            static_assert(Mode == AccessMode::ReadWrite, "can only flush mutable file buffers");

            if (const auto res = m_impl.FlushAsync(0, this->GetLength()); res != Success) {
                throw std::system_error(res, std::generic_category(), "failed to flush memory mapping to disk");
            }
        }
    };

    /// A read-only mapping of a file into memory.
    using ReadOnlyMapped  = MemoryMapped<AccessMode::ReadOnly>;
    /// A readable and writable mapping of a file into memory.
    using ReadWriteMapped = MemoryMapped<AccessMode::ReadWrite>;

}
