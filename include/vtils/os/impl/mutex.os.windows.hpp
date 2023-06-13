/**
 * @file mutex.os.windows.hpp
 * @brief Exclusive mutex implementation for Windows.
 * @copyright Valentin B.
 *
 * This implementation uses SRWLock instead of CriticalSection for the
 * following reasons:
 *
 * 1. SRWLock is several times faster than CriticalSection according to
 *    benchmarks performed on both Windows 8 and Windows 7.
 *
 * 2. CriticalSection allows recursive locking while SRWLock deadlocks.
 *    The UNIX implementation deadlocks so consistency is preferred.
 *
 * 3. We do not guarantee fairness in locking.
 *
 * 4. A `constexpr` constructor finally becomes feasible consistently.
 */
#pragma once

#include <memory>

#include <synchapi.h>

#include "vtils/macros/attr.hpp"

namespace vtils::impl {

    class CondVarImpl;

    class MutexImpl final {
        friend class CondVarImpl;

    private:
        SRWLOCK m_srw = SRWLOCK_INIT;

    private:
        ALWAYS_INLINE constexpr PSRWLOCK Raw() {
            return std::addressof(m_srw);
        }

    public:
        // SRWLocks can be statically initialized through the constant and
        // must not be explicitly destroyed or finalized.
        ALWAYS_INLINE constexpr MutexImpl() = default;
        ALWAYS_INLINE constexpr ~MutexImpl() = default;

        ALWAYS_INLINE void Initialize() {}
        ALWAYS_INLINE bool Finalize() { return false; }

        ALWAYS_INLINE void Lock() {
            ::AcquireSRWLockExclusive(std::addressof(m_srw));
        }

        ALWAYS_INLINE bool TryLock() {
            return ::TryAcquireSRWLockExclusive(std::addressof(m_srw)) != FALSE;
        }

        ALWAYS_INLINE void Unlock() {
            ::ReleaseSRWLockExclusive(std::addressof(m_srw));
        }
    };

}
