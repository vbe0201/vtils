/**
 * @file read_write_lock.hpp
 * @brief Reader-writer locks to protect shared data.
 * @copyright Valentin B.
 */
#pragma once

#include "vtils/macros/platform.hpp"

#if defined(V_PLATFORM_WINDOWS)
    #include "vtils/os/impl/read_write_lock.os.windows.hpp"
#else
    #error "Platform is currently not supported"
#endif

namespace vtils {

    namespace impl {

        /// @exclude
        class ReadWriteLock final {
        private:
            ReadWriteLockImpl m_impl;

        public:
            constexpr ReadWriteLock() = default;
            constexpr ~ReadWriteLock() = default;

            ReadWriteLock(const ReadWriteLock &) = delete;
            const ReadWriteLock &operator=(const ReadWriteLock &) = delete;

            ALWAYS_INLINE void Read() {
                m_impl.Read();
            }

            ALWAYS_INLINE bool TryRead() {
                return m_impl.TryRead();
            }

            ALWAYS_INLINE void ReadUnlock() {
                m_impl.ReadUnlock();
            }

            ALWAYS_INLINE void Write() {
                m_impl.Write();
            }

            ALWAYS_INLINE bool TryWrite() {
                return m_impl.TryWrite();
            }

            ALWAYS_INLINE void WriteUnlock() {
                m_impl.WriteUnlock();
            }
        };

    }

    // TODO: Public API.

}
