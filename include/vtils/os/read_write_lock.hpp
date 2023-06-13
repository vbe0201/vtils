/**
 * @file read_write_lock.hpp
 * @brief Reader-writer locks to protect shared data.
 * @copyright Valentin B.
 */
#pragma once

#include <optional>
#include <utility>

#include "vtils/macros/platform.hpp"
#include "vtils/os/impl/util_pointer_value.hpp"

#if defined(V_PLATFORM_WINDOWS)
    #include "vtils/os/impl/read_write_lock.os.windows.hpp"
#else
    #include "vtils/os/impl/read_write_lock.pthread.hpp"
#endif

namespace vtils {

    namespace impl {

        class ReadWriteLock final {
        private:
            PointerValue<ReadWriteLockImpl> m_impl;

        public:
            constexpr ReadWriteLock() = default;
            constexpr ~ReadWriteLock() = default;

            ReadWriteLock(const ReadWriteLock &) = delete;
            const ReadWriteLock &operator=(const ReadWriteLock &) = delete;

            ALWAYS_INLINE void Read() {
                m_impl.Get().Read();
            }

            ALWAYS_INLINE bool TryRead() {
                return m_impl.Get().TryRead();
            }

            ALWAYS_INLINE void ReadUnlock() {
                m_impl.Get().ReadUnlock();
            }

            ALWAYS_INLINE void Write() {
                m_impl.Get().Write();
            }

            ALWAYS_INLINE bool TryWrite() {
                return m_impl.Get().TryWrite();
            }

            ALWAYS_INLINE void WriteUnlock() {
                m_impl.Get().WriteUnlock();
            }
        };

    }

    template <typename T>
    struct ReaderGuard;

    template <typename T>
    struct WriterGuard;

    /// A synchronization primitive to protect shared data from being simultaneously
    /// accessed by multiple threads.
    ///
    /// It provides non-reentrant ownership semantics over a lock and the associated
    /// data it protects.
    ///
    /// This lock has two modes: shared and exclusive. In shared mode, many readers
    /// can simultaneously gain immutable access to the value. In exclusive mode,
    /// exactly one thread gains read and write access to the value.
    ///
    /// Since manual management of locks is often prone to errors, this one wraps
    /// the value it is intended to protect and only exposes it through a safe API
    /// which ensures proper resource management through RAII.
    ///
    /// @tparam T The type of data to guard.
    template <typename T>
    class ReadWriteLock {
        friend class ReaderGuard<T>;
        friend class WriterGuard<T>;

    public:
        // Copying locks is an error hazard since the point is to protect
        // a common shared value and not accidentally duplicate it.
        ReadWriteLock(const ReadWriteLock &) = delete;
        ReadWriteLock &operator=(const ReadWriteLock &) = delete;

    private:
        mutable impl::ReadWriteLock m_raw;
        T m_value;

    public:
        /// Default-constructs the ReadWriteLock along with its resource.
        ALWAYS_INLINE constexpr ReadWriteLock() : m_raw(), m_value() {}

        /// Constructs the ReadWriteLock from an existing resource.
        ALWAYS_INLINE constexpr explicit ReadWriteLock(T &&value)
            : m_raw(), m_value(std::forward<T>(value)) {}

        /// Constructs the ReadWriteLock with its resource in-place, forwarding all
        /// arguments to the resource constructor.
        template <typename... Args> requires std::is_constructible_v<T, Args...>
        ALWAYS_INLINE constexpr explicit ReadWriteLock(std::in_place_t, Args &&...args)
            : m_raw(), m_value(std::forward<Args>(args)...) {}

        /// Locks the lock for shared access, blocking the current thread until
        /// it becomes available.
        ///
        /// Shared access only grants immutable access to the resource, as there
        /// can be many other shared reader threads at the same time.
        ///
        /// @return A scope guard providing shared access to the resource.
        ///         When destructed, the resource will be released again.
        ALWAYS_INLINE ReaderGuard<T> Read() const;

        /// Attempts to lock the lock for shared access.
        ///
        /// If the lock is currently held exclusively, this will not wait for it
        /// to become available. Instead, the function returns immediately.
        ///
        /// @return A scope guard providing shared access to the resource on
        ///         success. Otherwise, an empty value.
        ALWAYS_INLINE std::optional<ReaderGuard<T>> TryRead() const;

        /// Locks the lock for exclusive access, blocking the current thread
        /// until it becomes available.
        ///
        /// Exclusive access grants both mutable and immutable access to the
        /// resource as there can only be one thread holding the lock at the
        /// same time.
        ///
        /// @return A scope guard providing exclusive access to the resource.
        ///         When destructed, the resource will be released again.
        ALWAYS_INLINE WriterGuard<T> Write();

        /// Attempts to lock the lock for exclusive access.
        ///
        /// If the lock is currently held shared or exclusively, this will not
        /// wait for it to become available. Instead, the function returns
        /// immediately.
        ///
        /// @return A scope guard providing exclusive access to the resource on
        ///         success. Otherwise, an empty value.
        ALWAYS_INLINE std::optional<WriterGuard<T>> TryWrite();
    };

    /// A lock guard providing RAII semantics for shared access to the value.
    ///
    /// When working with a guard object, its lifetime must never exceed that
    /// of the @ref ReadWriteLock it was obtained from.
    template <typename T>
    struct ReaderGuard {
        friend class ReadWriteLock<T>;

    public:
        // Guard is not copyable for the same reason as ReadWriteLock.
        ReaderGuard(const ReaderGuard &) = delete;
        ReaderGuard &operator=(const ReaderGuard &) = delete;

        // Guard is not movable because it represents a permit to access
        // the locked value on the current thread.
        ReaderGuard(ReaderGuard &&) = delete;
        ReaderGuard &operator=(ReaderGuard &&) = delete;

    private:
        const T *m_ptr;
        impl::ReadWriteLock &m_raw;

    private:
        ALWAYS_INLINE explicit ReaderGuard(const ReadWriteLock<T> &lock)
            : m_ptr(std::addressof(lock.m_value)), m_raw(lock.m_raw) {}

    public:
        // Releases exclusive access to the resource on destruction.
        ALWAYS_INLINE ~ReaderGuard() {
            m_raw.ReadUnlock();
        }

        // Immutable resource access, by pointer and by reference.
        ALWAYS_INLINE const T *operator->() const   { return m_ptr;  }
        ALWAYS_INLINE const T &operator*()  const & { return *m_ptr; }
    };

    /// A lock guard providing RAII semantics for exclusive access to the value.
    ///
    /// When working with a guard object, its lifetime must never exceed that
    /// of the @ref ReadWriteLock it was obtained from.
    template <typename T>
    struct WriterGuard {
        friend class ReadWriteLock<T>;

    public:
        // Guard is not copyable for the same reason as ReadWriteLock.
        WriterGuard(const WriterGuard &) = delete;
        WriterGuard &operator=(const WriterGuard &) = delete;

        // Guard is not movable because it represents a permit to access
        // the locked value on the current thread.
        WriterGuard(WriterGuard &&) = delete;
        WriterGuard &operator=(WriterGuard &&) = delete;

    private:
        T *m_ptr;
        impl::ReadWriteLock &m_raw;

    private:
        ALWAYS_INLINE explicit WriterGuard(ReadWriteLock<T> &lock)
            : m_ptr(std::addressof(lock.m_value)), m_raw(lock.m_raw) {}

    public:
        // Releases exclusive access to the resource on destruction.
        ALWAYS_INLINE ~WriterGuard() {
            m_raw.WriteUnlock();
        }

        // Immutable resource access, by pointer and by reference.
        ALWAYS_INLINE const T *operator->() const   { return m_ptr;  }
        ALWAYS_INLINE const T &operator*()  const & { return *m_ptr; }

        // Mutable resource access, by pointer and by reference.
        ALWAYS_INLINE T *operator->()   { return m_ptr;  }
        ALWAYS_INLINE T &operator*()  & { return *m_ptr; }
    };

    template<typename T>
    ReaderGuard<T> ReadWriteLock<T>::Read() const {
        m_raw.Read();
        return ReaderGuard(*this);
    }

    template<typename T>
    std::optional<ReaderGuard<T>> ReadWriteLock<T>::TryRead() const {
        if (!m_raw.TryRead()) {
            return {};
        }

        return ReaderGuard(*this);
    }

    template<typename T>
    WriterGuard<T> ReadWriteLock<T>::Write() {
        m_raw.Write();
        return WriterGuard(*this);
    }

    template<typename T>
    std::optional<WriterGuard<T>> ReadWriteLock<T>::TryWrite() {
        if (!m_raw.TryWrite()) {
            return {};
        }

        return WriterGuard(*this);
    }

}
