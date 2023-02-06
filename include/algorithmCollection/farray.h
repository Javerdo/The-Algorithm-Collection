#pragma once
#include <algorithm>
#include <memory>
#include <ranges>
#include <stdexcept>
#include <span>
#include "simpleAllocator.h"

// Traditional fixed-size array with memory safety
template <typename T, typename Alloc = SimpleAllocator<T>>
class Farray {
public:
    using iterator = T*;
    using const_iterator = const T*;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;
    using span = std::span<T>;
    using const_span = std::span<const T>;

    constexpr Farray() = default;
    constexpr Farray(std::initializer_list<T> values) {
        std::size_t index = 0;
        for (const auto& item : values) {
            if (index >= m_size) break;
            m_data[index++] = item;
        }
    }

    // Returns a reference to the element stored at the specified index in the array.
    T& operator[](std::size_t index) { return m_data[index]; }

    // Returns a constant reference to the element stored at the specified index in the array.
    const T& operator[](std::size_t index) const { return m_data[index]; }

    constexpr iterator begin() { return m_data.get(); }
    constexpr const_iterator begin() const { return m_data.get(); }
    constexpr const_iterator cbegin() const { return begin(); }
    constexpr iterator end() { return m_data.get() + m_size; }
    constexpr const_iterator end() const { return m_data.get() + m_size; }
    constexpr const_iterator cend() const { return end(); }
    constexpr reverse_iterator rbegin() { return reverse_iterator(end()); }
    constexpr const_reverse_iterator rbegin() const { return const_reverse_iterator(end()); }
    constexpr const_reverse_iterator crbegin() const { return rbegin(); }
    constexpr reverse_iterator rend() { return reverse_iterator(begin()); }
    constexpr const_reverse_iterator rend() const { return const_reverse_iterator(begin()); }
    constexpr const_reverse_iterator crend() const { return rend(); }

    // Returns a non-const span object that provides pointer access to the underlying stored data.
    constexpr span data() { return { m_data.get(), m_size }; }

    // Returns a const span object that provides read-only pointer access to the underlying stored data.
    constexpr const_span data() const { return { m_data.get(), m_size }; }

    // Returns size of array
    constexpr std::size_t size() const { return m_size; }

    // Checks whether array is empty or not
    constexpr bool empty() const { return m_size == 0; }

private:
    std::size_t m_size;
    std::unique_ptr<T[]> m_data;
    Alloc m_allocator;

    void allocate_memory(std::size_t size) {
        m_data = m_allocator.allocate(size);
        m_size = size;
    }

    void deallocate_memory() {
        m_allocator.deallocate(m_data, m_size);
    }
};