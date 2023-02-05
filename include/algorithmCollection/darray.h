#pragma once
#include <algorithm>
#include <memory>
#include <ranges>
#include <stdexcept>
#include <span>
#include <algorithmCollection/simpleAllocator.h>

// Dynamic array which increases size when at capacity
template <typename T, typename Alloc = SimpleAllocator<T>>
class Darray {
public:
    using iterator = T*;
    using const_iterator = const T*;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;
    using span = std::span<T>;
    using const_span = std::span<const T>;

    constexpr Darray() 
        : m_size(0), 
          m_capacity(0), 
          m_data(nullptr) {}

    constexpr explicit Darray(std::size_t size) 
        : m_size(size), 
          m_capacity(size), 
          m_data(std::make_unique<T[]>(size)) {}

    constexpr Darray(std::initializer_list<T> values) 
        : m_size(values.size()), 
          m_capacity(values.size()), 
          m_data(std::make_unique<T[]>(values.size())) {
        std::ranges::copy(values.begin(), values.end(), m_data.get());
    }

    constexpr Darray(const Darray& other) 
        : m_size(other.m_size), 
          m_capacity(other.m_size), 
          m_data(std::make_unique<T[]>(other.m_size)) {
        std::ranges::copy_n(other.m_data.get(), other.m_size, m_data.get());
    }

    constexpr Darray(Darray&& other) noexcept 
        : m_size(other.m_size), 
          m_capacity(other.m_capacity), 
          m_data(std::move(other.m_data)) {
        other.m_size = 0;
        other.m_capacity = 0;
    }

    // Overloading the assignment operator to copy data from another Darray object.
    Darray& operator=(const Darray& other) noexcept {
        if (this != &other) {
            m_size = other.m_size;
            m_capacity = other.m_capacity
            m_data = std::make_unique<T[]>(m_size);
            std::ranges::copy(other.m_data, m_data.get());
        }

        return *this;
    }

    // Overloading the assignment operator to handle move semantics
    Darray& operator=(Darray&& other) noexcept {
        m_size = other.m_size;
        m_capacity = other.m_capacity;
        m_data = std::move(other.m_data);
        other.m_size = 0;

        return *this;
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

    // Returns capacity of array
    constexpr std::size_t capacity() const { return m_capacity; }

    // Checks whether array is empty or not
    constexpr bool empty() const { return m_size == 0; }

    // Adds an element to the end of the array
    constexpr void push_back(const T& value) {
        if (m_size + 1 > m_capacity) {
            // Double the capacity of the array if m_size + 1 > m_capacity
            m_capacity = m_capacity == 0 ? 1 : m_capacity * 2;
            auto new_data = std::make_unique<T[]>(m_capacity);

            // Copy the elements from the current data to the new data
            std::ranges::copy_n(m_data.get(), m_size, new_data.get());
            m_data = std::move(new_data);
        }

        // Add the new value to the end of the new data
        m_data[m_size] = value;
        ++m_size;
    }

    // remove the last element from the array
    constexpr void pop_back() {
        // If array is empty, throw an error
        if (empty()) {
            throw std::logic_error("Array is empty");
        }

        std::size_t new_size = m_size -1;
        auto new_data = std::make_unique<T[]>(new_size);

        // Copy all elements except the last one from the original array to the new array
        std::ranges::copy_n(m_data.get(), std::min(m_size, new_size), new_data.get());
        m_data = std::move(new_data);
        m_size = new_size;
    }

    // Inserts an element at a specified index of the array (Reallocation if size increases)
    // - index: the index to insert the element at
    // - value: the value to insert
    constexpr void insert(std::size_t index, const T& value) {
        if (index > m_size) {
            throw std::out_of_range("Index out of range");
        }

        // Allocate more memory
        auto new_data = std::make_unique<T[]>(m_size + 1);

        std::copy(m_data.get(), m_data.get() + index, new_data.get());
        new_data[index] = value;

        // Copy the elements after the insertion point
        std::copy(m_data.get() + index, m_data.get() + m_size, new_data.get() + index + 1);
        m_data = std::move(new_data);
        ++m_size;
    }

    // Access element by index, throws if not within the bounds of the array.
    // - index: the index in array to access
    constexpr T& at(std::size_t index) const {
        if (index >= m_size) {
            throw std::out_of_range("Index out of range");
        }

        return m_data[index];
    }
 
    // Reserves storage space to store at least `new_capacity` elements in the array.
    // Note: If `new_capacity` is less than or equal to the current size of the custom array, the function does nothing.
    constexpr void reserve(std::size_t new_capacity) {
        if (new_capacity > m_capacity) {
            auto new_data = std::make_unique<T[]>(new_capacity);

            if (!new_data) {
                throw std::bad_alloc();
            }

            if (new_data.get() != m_data.get()) {
                std::ranges::copy(m_data, m_data.get() + m_size, new_data.get());
                m_data = std::move(new_data);
                m_capacity = new_capacity;
            }
        }
    }

private:
    std::size_t m_size;
    std::size_t m_capacity;
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