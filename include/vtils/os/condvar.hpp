/**
 * @file condvar.hpp
 * @brief Condition Variables for parking threads.
 * @copyright Valentin B.
 */
#pragma once

#include <type_traits>

#include "vtils/macros/platform.hpp"
#include "vtils/os/mutex.hpp"
#include "vtils/os/impl/util_pointer_value.hpp"

#if defined(V_PLATFORM_WINDOWS)
    #include "vtils/os/impl/condvar.os.windows.hpp"
#else
    #include "vtils/os/impl/condvar.pthread.hpp"
#endif

namespace vtils {

    namespace impl {

        template <typename T, class Fn>
        constexpr bool CondVarPredicate = std::is_invocable_r_v<bool, Fn, T&>;

        class CondVar final {
        private:
            PointerValue<CondVarImpl> m_impl;

        public:
            constexpr CondVar() = default;
            constexpr ~CondVar() = default;

            CondVar(const CondVar &) = delete;
            const CondVar &operator=(const CondVar &) = delete;

            ALWAYS_INLINE void Wait(Mutex &mutex) {
                m_impl.Get().Wait(mutex.GetImpl());
            }

            template <class Clock, class Duration>
            ALWAYS_INLINE bool WaitUntil(Mutex &mutex, const std::chrono::time_point<Clock, Duration> &time) {
                return m_impl.Get().WaitUntil(mutex.GetImpl(), time);
            }

            ALWAYS_INLINE void NotifyOne() {
                m_impl.Get().NotifyOne();
            }

            ALWAYS_INLINE void NotifyAll() {
                m_impl.Get().NotifyAll();
            }
        };

    }

    class ConditionVariable {
    public:
        // Copying condition variables is an error hazard since the point
        // is to synchronize between waiters on the same variable.
        ConditionVariable(const ConditionVariable &) = delete;
        ConditionVariable &operator=(const ConditionVariable &) = delete;

    private:
        impl::CondVar m_raw;

    public:
        /// Constructs a new condition variable.
        ALWAYS_INLINE constexpr ConditionVariable() : m_raw() {}

        /// Wakes up one blocked thread on this condition variable.
        ///
        /// Calls to NotifyOne() are not buffered, so only threads
        /// currently waiting will receive the notification.
        ALWAYS_INLINE void NotifyOne() {
            m_raw.NotifyOne();
        }

        /// Notifies all blocked threads on this condition variable.
        ///
        /// Calls to NotifyAll() are not buffered, all currently
        /// waiting threads will receive the notification.
        ALWAYS_INLINE void NotifyAll() {
            m_raw.NotifyAll();
        }

        /// Blocks the current thread on the condition variable until
        /// it receives a notification.
        ///
        /// The function will atomically unlock the provided mutex and
        /// re-acquire it on wakeup.
        ///
        /// Note that this function is susceptible to spurious wakeups.
        /// Condition variables usually have a predicate associated
        /// with them which must be checked on every wakeup.
        ///
        /// This function aborts execution when used with two different
        /// mutexes at any given point.
        ///
        /// @param guard The guard of the locked mutex.
        template <typename T>
        ALWAYS_INLINE void Wait(MutexGuard<T> &guard) {
            m_raw.Wait(guard.m_raw);
        }

        /// Blocks the current thread on the condition variable until
        /// it receives a notification and the given predicate is true.
        ///
        /// The function will atomically unlock the provided mutex and
        /// re-acquire it on wakeup.
        ///
        /// Note that unlike @ref Wait, this function is not susceptible
        /// to spurious wakeups due to waiting on a predicate condition.
        ///
        /// This function aborts execution when used with two different
        /// mutexes at any given point.
        ///
        /// @param guard The guard of the locked mutex.
        /// @param pred A predicate to wait for before returning.
        template <typename T, class Fn> requires impl::CondVarPredicate<T, Fn>
        void Wait(MutexGuard<T> &guard, Fn pred) {
            while (!pred(*guard)) {
                this->Wait(guard);
            }
        }

        /// Blocks the current thread on the condition variable until
        /// it receives a notification or times out.
        ///
        /// The function will automatically unlock the provided mutex
        /// and re-acquire it on wakeup.
        ///
        /// Note that this function is susceptible to spurious wakeups.
        /// Condition variables usually have a predicate associated
        /// with them which must be checked on every wakeup.
        ///
        /// This function aborts execution when used with two different
        /// mutexes at any given point.
        ///
        /// @param guard The guard of the locked mutex.
        /// @param time The duration to wait for.
        /// @return `true` on success, `false` on timeout.
        template <typename T, class Rep, class Period>
        bool WaitFor(MutexGuard<T> &guard, const std::chrono::duration<Rep, Period> &time) {
            auto end = std::chrono::steady_clock::now() + time;
            return m_raw.WaitUntil(guard.m_raw, end);
        }

        /// Blocks the current thread on the condition variable until
        /// it receives a notification and its predicate is true, or
        /// times out.
        ///
        /// The function will automatically unlock the provided mutex
        /// and re-acquire it on wakeup.
        ///
        /// Note that this function is not susceptible to spurious
        /// wakeups due to waiting on a predicate condition.
        ///
        /// This function aborts execution when used with two different
        /// mutexes at any given point.
        ///
        /// @param guard The guard of the locked mutex.
        /// @param time The duration to wait for.
        /// @param pred A predicate to wait for before returning.
        /// @return `true` on success, `false` on timeout.
        template <typename T, class Rep, class Period, class Fn> requires impl::CondVarPredicate<T, Fn>
        bool WaitFor(MutexGuard<T> &guard, const std::chrono::duration<Rep, Period> &time, Fn pred) {
            auto end = std::chrono::steady_clock::now() + time;
            while (!pred(*guard)) {
                if (!m_raw.WaitUntil(guard.m_raw, end)) {
                    return pred(*guard);
                }
            }

            return true;
        }
    };

}
