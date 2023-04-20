#pragma once
#include <utility>

template <typename T>
class SimpleAllocator {
public:
    using value_type = T;

    SimpleAllocator() = default;
    SimpleAllocator(const SimpleAllocator&) = default;

    T* allocate(std::size_t n) {
        if (n == 0) {
            return nullptr;
        }
        return static_cast<T*>(::operator new(n * sizeof(T)));
    }

    T* allocate(std::size_t n) const {
        if (n == 0) {
            return nullptr;
        }
        return static_cast<T*>(::operator new(n * sizeof(T)));
    }

    void deallocate(T* p, std::size_t n) {
        static_cast<void>(n); // Silence the -werror=unused warning. TODO: use the parameter properly.
        if (p) {
            std::free(p);
        }
    }

    template <typename U, typename... Args>
    void construct(U* p, Args&&... args) {
        new (p) U(std::forward<Args>(args)...);
    }

    template <typename U>
    void construct(U* p, const T& value) {
        new (p) U(value);
    }

    void destroy(T* p) {
        p->~T();
    }
};