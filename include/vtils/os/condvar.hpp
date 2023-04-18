/**
 * @file condvar.hpp
 * @brief Condition Variables for parking threads.
 * @copyright Valentin B.
 */
#pragma once

#include "vtils/macros/platform.hpp"
#include "vtils/os/mutex.hpp"

#if defined(V_PLATFORM_WINDOWS)
    #include "vtils/os/impl/condvar.os.windows.hpp"
#else
    #error "Platform is currently not supported"
#endif

namespace vtils {

    namespace impl {

        /// @exclude
        class CondVar final {
        private:
            CondVarImpl m_impl;

        public:
            constexpr CondVar() = default;
            constexpr ~CondVar() = default;

            CondVar(const CondVar &) = delete;
            const CondVar &operator=(const CondVar &) = delete;

            ALWAYS_INLINE void Wait(Mutex &mutex) {
                m_impl.Wait(mutex.GetImpl());
            }

            ALWAYS_INLINE bool WaitTimeout(Mutex &mutex, std::chrono::milliseconds ms) {
                return m_impl.WaitTimeout(mutex.GetImpl(), ms);
            }

            ALWAYS_INLINE void NotifyOne() {
                m_impl.NotifyOne();
            }

            ALWAYS_INLINE void NotifyAll() {
                m_impl.NotifyAll();
            }
        };

    }

    // TODO: Public API.

}
