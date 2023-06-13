/**
 * @file read_write_lock.pthread.hpp
 * @brief Reader-Writer lock implementation based on pthreads.
 * @copyright Valentin B.
 */
#pragma once

#include <memory>

#include <pthread.h>

#include "vtils/assert.hpp"
#include "vtils/macros/platform.hpp"

namespace vtils::impl {

    class ReadWriteLockImpl final {
    private:
        pthread_rwlock_t m_rwlock = PTHREAD_RWLOCK_INITIALIZER;

    private:
        ALWAYS_INLINE void RawUnlock() {
            int res = pthread_rwlock_unlock(std::addressof(m_rwlock));
            V_DEBUG_ASSERT(res == 0);
        }

    public:
        constexpr ReadWriteLockImpl() = default;
        constexpr ~ReadWriteLockImpl() = default;

        ALWAYS_INLINE void Initialize() {}

        ALWAYS_INLINE bool Finalize() {
            // We're not allowed to destroy a locked lock, so check first.
            if (this->TryWrite()) {
                this->WriteUnlock();

                int res = pthread_rwlock_destroy(std::addressof(m_rwlock));
                #if defined(V_PLATFORM_DRAGONFLY)
                // On DragonFly pthread_rwlock_destroy() returns EINVAL if called
                // on a lock that was just constant-initialized. Once used
                // or pthread_rwlock_init() is called, this no longer occurs.
                V_DEBUG_ASSERT(res == 0 || res == EINVAL);
                #else
                V_DEBUG_ASSERT(res == 0);
                #endif

                return true;
            }

            return false;
        }

        ALWAYS_INLINE void Read() {
            int res = pthread_rwlock_rdlock(std::addressof(m_rwlock));

            if (res == EAGAIN) {
                V_ABORT("rwlock maximum reader count exceeded");
            } else if (res == EDEADLK) {
                V_ABORT("rwlock read lock would result in deadlock");
            } else {
                // POSIX does not make guarantees about all errors that may be returned.
                V_ASSERT(res == 0, "unexpected error during rwlock read lock: {}", res);
            }
        }

        ALWAYS_INLINE bool TryRead() {
            int res = pthread_rwlock_tryrdlock(std::addressof(m_rwlock));

            if (res == EBUSY || res == EAGAIN) {
                return false;
            }

            V_DEBUG_ASSERT(res == 0);
            return true;
        }

        ALWAYS_INLINE void Write() {
            int res = pthread_rwlock_wrlock(std::addressof(m_rwlock));

            if (res == EDEADLK) {
                V_ABORT("rwlock write lock would result in deadlock");
            }

            V_DEBUG_ASSERT(res == 0);
        }

        ALWAYS_INLINE bool TryWrite() {
            int res = pthread_rwlock_trywrlock(std::addressof(m_rwlock));

            if (res == EBUSY) {
                return false;
            }

            V_DEBUG_ASSERT(res == 0);
            return true;
        }

        ALWAYS_INLINE void ReadUnlock() {
            this->RawUnlock();
        }

        ALWAYS_INLINE void WriteUnlock() {
            this->RawUnlock();
        }
    };

}
