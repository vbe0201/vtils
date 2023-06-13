/**
 * @file lazy_box.hpp
 * @brief Utility for lazy initialization and boxing too large OS types.
 * @copyright Valentin B.
 */
#pragma once

#include <atomic>
#include <memory>
#include <concepts>
#include <type_traits>

#include <stdint.h>

#include "vtils/assert.hpp"

namespace vtils::impl {

    template <typename T>
    concept RuntimeInitializable = requires (T &t) {
        { t.Initialize() } -> std::same_as<void>;
        { t.Finalize()   } -> std::same_as<bool>;
    };

    template <typename T>
    ALWAYS_INLINE constexpr bool UseInlineStorageForPointerValue() {
        return sizeof(T) <= sizeof(T*);
    }

    template <typename T, typename = void>
    struct PointerValueStorage;

    // If the value already fits a pointer in size, wrap it directly.
    template <typename T>
    struct PointerValueStorage<T, std::enable_if_t<UseInlineStorageForPointerValue<T>()>> {
    private:
        T m_inner;

    public:
        ALWAYS_INLINE constexpr PointerValueStorage() : m_inner() {}
        ALWAYS_INLINE constexpr ~PointerValueStorage() {}

        ALWAYS_INLINE T *GetPointer() {
            return std::addressof(m_inner);
        }
    };

    // If the value does not fit a pointer in size, leverage `RuntimeInitializable`
    // to initialize a heap-allocated value on first access at runtime and store
    // the pointer to it.
    template <typename T> requires RuntimeInitializable<T>
    struct PointerValueStorage<T, std::enable_if_t<!UseInlineStorageForPointerValue<T>()>> {
    private:
        std::atomic<T*> m_inner;

    private:
        COLD T *Initialize() {
            T *ptr = new (std::nothrow) T();
            V_ASSERT(ptr != nullptr);

            T *res = nullptr;
            if (m_inner.compare_exchange_strong(res, ptr, std::memory_order_acq_rel, std::memory_order_acquire)) {
                // This thread successfully initialized the storage.
                ptr->Initialize();
                return ptr;
            } else {
                // We raced with another thread which already allocated the storage.
                // Free our heap allocation and return the existing object pointer.
                delete ptr;
                return res;
            }
        }

    public:
        ALWAYS_INLINE constexpr PointerValueStorage() : m_inner(nullptr) {}

        ALWAYS_INLINE constexpr ~PointerValueStorage() {
            if (!std::is_constant_evaluated()) {
                T *ptr = m_inner.load(std::memory_order_relaxed);
                if (ptr != nullptr && ptr->Finalize()) {
                    delete ptr;
                }
            }
        }

        ALWAYS_INLINE T *GetPointer() {
            T *ptr = m_inner.load(std::memory_order_acquire);
            if (ptr == nullptr) {
                return this->Initialize();
            } else {
                return ptr;
            }
        }
    };

    template <typename T> requires RuntimeInitializable<T>
    class PointerValue final {
    private:
        PointerValueStorage<T> m_storage;

    public:
        ALWAYS_INLINE constexpr PointerValue() : m_storage() {}
        ALWAYS_INLINE constexpr ~PointerValue() {}

        ALWAYS_INLINE T &Get() {
            return *m_storage.GetPointer();
        }
    };

}
