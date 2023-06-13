/**
 * @brief condvar.os.windows.hpp
 * @brief Condition Variable implementation for Windows.
 * @copyright Valentin B.
 */
#pragma once

#include <chrono>
#include <limits>
#include <memory>

#include <errhandlingapi.h>
#include <synchapi.h>
#include <winerror.h>

#include "vtils/assert.hpp"
#include "vtils/macros/attr.hpp"
#include "vtils/os/impl/mutex.os.windows.hpp"

namespace vtils::impl {

    constexpr inline DWORD WindowsInfinite = std::numeric_limits<DWORD>::max();

    class CondVarImpl final {
    private:
        CONDITION_VARIABLE m_cond = CONDITION_VARIABLE_INIT;

    public:
        // CondVars can be statically initialized through the constant and
        // must not be explicitly destroyed or finalized.
        ALWAYS_INLINE constexpr CondVarImpl() = default;
        ALWAYS_INLINE constexpr ~CondVarImpl() = default;

        ALWAYS_INLINE void Initialize() {}
        ALWAYS_INLINE bool Finalize() { return false; }

        ALWAYS_INLINE void Wait(MutexImpl &mutex) {
            auto res = ::SleepConditionVariableSRW(std::addressof(m_cond), mutex.Raw(), WindowsInfinite, 0);
            V_DEBUG_ASSERT(res != 0);
        }

        template <class Clock, class Duration>
        ALWAYS_INLINE bool WaitUntil(MutexImpl &mutex, const std::chrono::time_point<Clock, Duration> &time) {
            auto reltime = time - std::chrono::system_clock::now();

            auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(reltime);
            auto duration = std::min(ms.count(), static_cast<std::chrono::milliseconds::rep>(WindowsInfinite));

            if (::SleepConditionVariableSRW(std::addressof(m_cond), mutex.Raw(), static_cast<DWORD>(duration), 0) == 0) {
                V_DEBUG_ASSERT(::GetLastError() == ERROR_TIMEOUT);
                return false;
            } else {
                return true;
            }
        }

        ALWAYS_INLINE void NotifyOne() {
            ::WakeConditionVariable(std::addressof(m_cond));
        }

        ALWAYS_INLINE void NotifyAll() {
            ::WakeAllConditionVariable(std::addressof(m_cond));
        }
    };

}
