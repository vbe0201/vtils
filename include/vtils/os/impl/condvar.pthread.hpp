/**
 * @brief condvar.pthread.hpp
 * @brief Condition Variable implementation on top of pthreads.
 * @copyright Valentin B.
 */
#pragma once

#include <atomic>
#include <chrono>

#include <pthread.h>

#include "vtils/assert.hpp"
#include "vtils/scope_guard.hpp"
#include "vtils/macros/platform.hpp"
#include "vtils/os/impl/mutex.pthread.hpp"

namespace vtils::impl {

    using std::chrono::duration;
    using std::chrono::duration_cast;
    using std::chrono::nanoseconds;
    using std::chrono::seconds;
    using std::chrono::time_point_cast;
    using std::chrono::years;

    class CondVarImpl final {
    private:
        pthread_cond_t m_cond = PTHREAD_COND_INITIALIZER;
        std::atomic<pthread_mutex_t *> m_mutex;

    private:
        ALWAYS_INLINE void Verify(pthread_mutex_t *mutex) {
            pthread_mutex_t *tmp = nullptr;
            // Relaxed is fine because we only use it to compare addresses.
            if (!m_mutex.compare_exchange_strong(tmp, mutex, std::memory_order_relaxed, std::memory_order_relaxed)) {
                V_ASSERT(tmp == mutex, "attempted to use condvar with two mutexes");
            }
        }

    public:
        constexpr CondVarImpl() = default;
        constexpr ~CondVarImpl() = default;

        ALWAYS_INLINE void Initialize() {
            #if !defined(V_PLATFORM_APPLE) && !defined(V_PLATFORM_ANDROID)
            // `pthread_condattr_setclock` is not supported on Apple and old Android.
            pthread_condattr_t attr;
            V_ASSERT(pthread_condattr_init(std::addressof(attr)) == 0);
            V_ON_SCOPE_EXIT {
                int res = pthread_condattr_destroy(std::addressof(attr));
                V_DEBUG_ASSERT(res == 0);
            };

            V_ASSERT(pthread_condattr_setclock(std::addressof(attr), CLOCK_MONOTONIC) == 0);
            V_ASSERT(pthread_cond_init(std::addressof(m_cond), std::addressof(attr)) == 0);
            #endif
        }

        ALWAYS_INLINE bool Finalize() {
            int res = pthread_cond_destroy(std::addressof(m_cond));
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

        ALWAYS_INLINE void Wait(MutexImpl &mutex) {
            auto *raw = mutex.Raw();

            this->Verify(raw);
            int res = pthread_cond_wait(std::addressof(m_cond), raw);
            V_DEBUG_ASSERT(res == 0);
        }

        template <class Clock, class Duration>
        ALWAYS_INLINE bool WaitUntil(MutexImpl &mutex, const std::chrono::time_point<Clock, Duration> &time) {
            auto *raw = mutex.Raw();
            this->Verify(raw);

            auto secs = impl::time_point_cast<impl::seconds>(time);
            auto ns   = impl::duration_cast<impl::nanoseconds>(time - secs);

            struct timespec ts = {
                .tv_sec  = static_cast<time_t>(secs.time_since_epoch().count()),
                .tv_nsec = static_cast<long>(ns.count()),
            };

            #if !defined(V_PLATFORM_APPLE) && !defined(V_PLATFORM_ANDROID)
            int res = pthread_cond_timedwait(std::addressof(m_cond), raw, std::addressof(ts));
            #else
            int res = pthread_cond_timedwait_relative_np(std::addressof(m_cond), raw, std::addressof(ts));
            #endif
            V_ASSERT(res == 0 || res == ETIMEDOUT);

            return Clock::now() < time;
        }

        ALWAYS_INLINE void NotifyOne() {
            int res = pthread_cond_signal(std::addressof(m_cond));
            V_DEBUG_ASSERT(res == 0);
        }

        ALWAYS_INLINE void NotifyAll() {
            int res = pthread_cond_broadcast(std::addressof(m_cond));
            V_DEBUG_ASSERT(res == 0);
        }
    };

}
