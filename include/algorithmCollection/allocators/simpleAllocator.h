#pragma once

// Simple Allocator
template <typename T>
class SimpleAllocator {
    public:
    using value_type = T;

    T* allocate(std::size_t n) {
        return static_cast<T*>(malloc(n * sizeof(T)));
    }

    void deallocate(T* p, std::size_t n) {
        free(p);
    }

    template <typename U, typename... Args>
    void construct(U* p, Args&&... args) {
        new (p) U(std::forward<Args>(args)...);
    }

    void destroy(T* p) {
        p->~T();
    }
};