/**
 * @file mutex.hpp
 * @brief Mutual exclusion primitive to protect shared data.
 * @copyright Valentin B.
 */
#pragma once

#include "vtils/macros/platform.hpp"

#if defined(V_PLATFORM_WINDOWS)
    #include "vtils/os/impl/mutex.os.windows.hpp"
#else
    #error "Platform is currently not supported"
#endif

namespace vtils {

    namespace impl {

        class CondVar;

        /// @exclude
        class Mutex final {
            friend class CondVar;

        private:
            MutexImpl m_impl;

        private:
            ALWAYS_INLINE constexpr MutexImpl &GetImpl() {
                return m_impl;
            }

        public:
            constexpr Mutex() = default;
            constexpr ~Mutex() = default;

            Mutex(const Mutex &) = delete;
            const Mutex &operator=(const Mutex &) = delete;

            ALWAYS_INLINE void Lock() {
                m_impl.Lock();
            }

            ALWAYS_INLINE bool TryLock() {
                return m_impl.TryLock();
            }

            ALWAYS_INLINE void Unlock() {
                m_impl.Unlock();
            }
        };

    }

    // TODO: Public API.

}
