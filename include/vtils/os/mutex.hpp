/**
 * @file mutex.hpp
 * @brief Mutual exclusion primitive to protect shared data.
 * @copyright Valentin B.
 */
#pragma once

#include <optional>
#include <utility>

#include "vtils/macros/platform.hpp"
#include "vtils/os/impl/util_pointer_value.hpp"

#if defined(V_PLATFORM_WINDOWS)
    #include "vtils/os/impl/mutex.os.windows.hpp"
#else
    #include "vtils/os/impl/mutex.pthread.hpp"
#endif

namespace vtils {

    namespace impl {

        class CondVar;

        class Mutex final {
            friend class CondVar;

        private:
            PointerValue<MutexImpl> m_impl;

        private:
            ALWAYS_INLINE MutexImpl &GetImpl() {
                return m_impl.Get();
            }

        public:
            constexpr Mutex() = default;
            constexpr ~Mutex() = default;

            Mutex(const Mutex &) = delete;
            const Mutex &operator=(const Mutex &) = delete;

            ALWAYS_INLINE void Lock() {
                m_impl.Get().Lock();
            }

            ALWAYS_INLINE bool TryLock() {
                return m_impl.Get().TryLock();
            }

            ALWAYS_INLINE void Unlock() {
                m_impl.Get().Unlock();
            }
        };

    }

    class ConditionVariable;

    template <typename T>
    struct MutexGuard;

    /// A mutual exclusion primitive to protect shared data from being simultaneously
    /// accessed by multiple threads.
    ///
    /// It provides non-reentrant ownership semantics over a lock and the associated
    /// data it protects.
    ///
    /// Since manual management of mutexes is often prone to errors, this one wraps
    /// the value it is intended to protect and only exposes it through a safe API
    /// which ensures proper resource management through RAII.
    ///
    /// @tparam T The type of data to guard.
    template <typename T>
    class Mutex {
        friend class MutexGuard<T>;

    public:
        // Copying mutexes is an error hazard since the point is to protect
        // a common shared value and not accidentally duplicate it.
        Mutex(const Mutex &) = delete;
        Mutex &operator=(const Mutex &) = delete;

    private:
        impl::Mutex m_raw;
        T m_value;

    public:
        /// Default-constructs the Mutex along with its resource.
        ALWAYS_INLINE constexpr Mutex() : m_raw(), m_value() {}

        /// Constructs the Mutex from an existing resource.
        ALWAYS_INLINE constexpr explicit Mutex(T &&value)
            : m_raw(), m_value(std::forward<T>(value)) {}

        /// Constructs the Mutex with its resource in-place, forwarding all arguments
        /// to the resource constructor.
        template <typename... Args> requires std::is_constructible_v<T, Args...>
        ALWAYS_INLINE constexpr explicit Mutex(std::in_place_t, Args &&...args)
            : m_raw(), m_value(std::forward<Args>(args)...) {}

        /// Locks the Mutex, blocking the current thread until it becomes available.
        ///
        /// @return A scope guard providing exclusive access to the resource. When
        ///         destructed, the resource will be released again.
        ALWAYS_INLINE MutexGuard<T> Lock();

        /// Attempts to lock the Mutex, returning a @ref MutexGuard on success.
        ///
        /// If the lock is currently held, this will not wait for it to
        /// become available and return immediately.
        ///
        /// @return When locked, a scope guard providing exclusive access to the
        ///         resource. Otherwise, an empty value.
        ALWAYS_INLINE std::optional<MutexGuard<T>> TryLock();
    };

    /// A lock guard providing RAII semantics for accessing the value.
    ///
    /// When working with a guard object, its lifetime must never exceed
    /// that of the @ref Mutex it was obtained from.
    template <typename T>
    struct MutexGuard {
        friend class ConditionVariable;
        friend class Mutex<T>;

    public:
        // Guard is not copyable for the same reason as Mutex.
        MutexGuard(const MutexGuard &) = delete;
        MutexGuard &operator=(const MutexGuard &) = delete;

        // Guard is not movable because it represents an exclusive
        // permit to access the locked value on the current thread.
        MutexGuard(MutexGuard &&) = delete;
        MutexGuard &operator=(MutexGuard &&) = delete;

    private:
        T *m_ptr;
        impl::Mutex &m_raw;

    private:
        ALWAYS_INLINE explicit MutexGuard(Mutex<T> &m)
            : m_ptr(std::addressof(m.m_value)), m_raw(m.m_raw) {}

    public:
        // Releases exclusive access to the resource on destruction.
        ALWAYS_INLINE ~MutexGuard() {
            m_raw.Unlock();
        }

        // Immutable resource access, by pointer and by reference.
        ALWAYS_INLINE const T *operator->() const   { return m_ptr;  }
        ALWAYS_INLINE const T &operator*()  const & { return *m_ptr; }

        // Mutable resource access, by pointer and by reference.
        ALWAYS_INLINE T *operator->()   { return m_ptr;  }
        ALWAYS_INLINE T &operator*()  & { return *m_ptr; }
    };

    template<typename T>
    MutexGuard<T> Mutex<T>::Lock() {
        m_raw.Lock();
        return MutexGuard(*this);
    }

    template<typename T>
    std::optional<MutexGuard<T>> Mutex<T>::TryLock() {
        if (!m_raw.TryLock()) {
            return {};
        }

        return MutexGuard(*this);
    }

}
