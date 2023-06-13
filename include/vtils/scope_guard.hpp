/**
 * @file scope_guard.hpp
 * @brief Scope guards for automatic deferred resource cleanup.
 * @copyright Valentin B.
 */
#pragma once

#include <type_traits>
#include <utility>

#include "vtils/macros/attr.hpp"
#include "vtils/macros/misc.hpp"

namespace vtils::impl {

    struct ScopeGuardToken {};

    template <class Fn>
    class ScopeGuard final {
    private:
        Fn m_fn;
        bool m_active;

    public:
        ALWAYS_INLINE constexpr explicit ScopeGuard(Fn fn) : m_fn(std::move(fn)), m_active(true) {}

        ALWAYS_INLINE constexpr ~ScopeGuard() {
            if (m_active) {
                m_fn();
            }
        }

        ALWAYS_INLINE constexpr void Cancel() {
            m_active = false;
        }

        ScopeGuard(const ScopeGuard &) = delete;

        ALWAYS_INLINE constexpr ScopeGuard(ScopeGuard &&rhs)
            : m_fn(std::move(rhs.m_fn)), m_active(rhs.m_active)
        {
            rhs.Cancel();
        }

        ScopeGuard &operator=(ScopeGuard &&) = delete;
    };

    // `->*` has the highest precedence of operators we can use for this.

    template <class Fn>
    ALWAYS_INLINE static constexpr ScopeGuard<Fn> operator->*(ScopeGuardToken, Fn &&fn) {
        return ScopeGuard<std::decay_t<Fn>>(std::forward<Fn>(fn));
    }

}

#define V_IMPL_SCOPE_GUARD(__NAME__, __TOKEN__) \
    auto V_ANON_VAR(__NAME__) = ::vtils::impl::__TOKEN__{}->*[&]() ALWAYS_INLINE_LAMBDA

/// Creates a scope guard which runs a block of code when leaving the scope.
///
/// The primary use case of this is deferred cleanup of resources not modeled
/// after RAII principles. Keeping the scope guard close to resource creation
/// ensures a resource is never forgotten or freed too eagerly.
#define V_ON_SCOPE_EXIT V_IMPL_SCOPE_GUARD(__V_IMPL_SCOPE_EXIT_GUARD__, ScopeGuardToken)
