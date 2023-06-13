/**
 * @file mutex.pthread.hpp
 * @brief Exclusive mutex implementation based on pthreads.
 * @copyright Valentin B.
 */
#pragma once

#include <memory>

#include <pthread.h>

#include "vtils/assert.hpp"
#include "vtils/scope_guard.hpp"
#include "vtils/macros/attr.hpp"
#include "vtils/macros/platform.hpp"

namespace vtils::impl {

    class CondVarImpl;

    class MutexImpl final {
        friend class CondVarImpl;

    private:
        pthread_mutex_t m_mutex = PTHREAD_MUTEX_INITIALIZER;

    private:
        ALWAYS_INLINE constexpr pthread_mutex_t *Raw() {
            return std::addressof(m_mutex);
        }

    public:
        constexpr MutexImpl() = default;
        constexpr ~MutexImpl() = default;

        ALWAYS_INLINE void Initialize() {
            pthread_mutexattr_t attr;
            V_ASSERT(pthread_mutexattr_init(std::addressof(attr)) == 0);
            V_ON_SCOPE_EXIT {
                int res = pthread_mutexattr_destroy(std::addressof(attr));
                V_DEBUG_ASSERT(res == 0);
            };

            V_ASSERT(pthread_mutexattr_settype(std::addressof(attr), PTHREAD_MUTEX_NORMAL) == 0);
            V_ASSERT(pthread_mutex_init(std::addressof(m_mutex), std::addressof(attr)) == 0);
        }

        ALWAYS_INLINE bool Finalize() {
            // We must not destroy a locked mutex, so check first.
            // Otherwise we just leak the memory because not much we can do.
            if (pthread_mutex_trylock(std::addressof(m_mutex)) == 0) {
                pthread_mutex_unlock(std::addressof(m_mutex));

                int res = pthread_mutex_destroy(std::addressof(m_mutex));
                #if defined(V_PLATFORM_DRAGONFLY)
                // On DragonFly pthread_cond_destroy() returns EINVAL if called
                // on a condvar that was just constant-initialized. Once used
                // or pthread_cond_init() is called, this no longer occurs.
                V_DEBUG_ASSERT(res == 0 || res == EINVAL);
                #else
                V_DEBUG_ASSERT(res == 0);
                #endif

                return true;
            }

            return false;
        }

        ALWAYS_INLINE void Lock() {
            int res = pthread_mutex_lock(std::addressof(m_mutex));
            V_DEBUG_ASSERT(res == 0);
        }

        ALWAYS_INLINE bool TryLock() {
            return pthread_mutex_trylock(std::addressof(m_mutex)) == 0;
        }

        ALWAYS_INLINE void Unlock() {
            int res = pthread_mutex_unlock(std::addressof(m_mutex));
            V_DEBUG_ASSERT(res == 0);
        }
    };

}
