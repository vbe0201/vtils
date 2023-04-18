/**
 * @file read_write_lock.os.windows.hpp
 * @brief Reader-Writer lock implementation for Windows.
 * @copyright Valentin B.
 */
#pragma once

#include <memory>

#include <synchapi.h>

#include "vtils/macros/attr.hpp"

namespace vtils::impl {

    /// @exclude
    class ReadWriteLockImpl final {
    private:
        SRWLOCK m_srw = SRWLOCK_INIT;

    public:
        // SRWLocks can be statically initialized through the constant and
        // must not be explicitly destroyed or finalized.
        ALWAYS_INLINE constexpr ReadWriteLockImpl() = default;
        ALWAYS_INLINE constexpr ~ReadWriteLockImpl() = default;

        ALWAYS_INLINE void Read() {
            ::AcquireSRWLockShared(std::addressof(m_srw));
        }

        ALWAYS_INLINE bool TryRead() {
            return ::TryAcquireSRWLockShared(std::addressof(m_srw)) != FALSE;
        }

        ALWAYS_INLINE void Write() {
            ::AcquireSRWLockExclusive(std::addressof(m_srw));
        }

        ALWAYS_INLINE bool TryWrite() {
            return ::TryAcquireSRWLockExclusive(std::addressof(m_srw)) != FALSE;
        }

        ALWAYS_INLINE void ReadUnlock() {
            ::ReleaseSRWLockShared(std::addressof(m_srw));
        }

        ALWAYS_INLINE void WriteUnlock() {
            ::ReleaseSRWLockExclusive(std::addressof(m_srw));
        }
    };

}
