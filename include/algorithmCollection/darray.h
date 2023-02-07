#pragma once
#include <algorithm>
#include <memory>
#include <ranges>
#include <stdexcept>
#include <span>
#include <cstring>
#include "simpleAllocator.h"

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

    constexpr explicit Darray(std::span<T> values)
        : m_size(values.size()),
        m_capacity(values.size()),
        m_data(std::make_unique<T[]>(values.size())) {
        std::copy(values.begin(), values.end(), m_data.get());
    }

    constexpr Darray(std::initializer_list<T> values)
        : m_size(values.size()),
        m_capacity(values.size()),
        m_data(std::make_unique<T[]>(values.size())) {
        std::copy(values.begin(), values.end(), m_data.get());
    }

    constexpr Darray(const Darray& other)
        : m_size(other.m_size),
        m_capacity(other.m_size),
        m_data(std::make_unique<T[]>(other.m_size)) {
        std::copy_n(other.m_data.get(), other.m_size, m_data.get());
    }

    constexpr Darray(Darray&& other) noexcept
        : m_size(other.m_size),
        m_capacity(other.m_capacity),
        m_data(std::move(other.m_data)) {
        other.m_size = 0;
        other.m_capacity = 0;
    }

    // Overloading the copy assignment operator to copy data from another Darray object.
    constexpr Darray& operator=(const Darray& other) noexcept {
        if (this != &other) {
            m_size = other.m_size;
            m_capacity = other.m_capacity;
            m_data = std::make_unique<T[]>(m_size);
            if constexpr (std::is_trivially_copyable_v<T>) {
                std::memcpy(m_data.get(), other.m_data.get(), m_size * sizeof(T));
            }
            else {
                std::copy(other.begin(), other.end(), m_data.get());
            }
        }

        return *this;
    }

    // Overloading the assignment operator to handle move semantics
    constexpr Darray& operator=(Darray&& other) noexcept {
        m_size = other.m_size;
        m_capacity = other.m_capacity;
        m_data = std::move(other.m_data);
        other.m_size = 0;

        return *this;
    }

    /*
    constexpr bool operator==(const MyArray<T>& other) const {
        const_span thisData = data();
        const_span otherData = other.data();

        return thisData == otherData;
    } */

    // == operator on data
    constexpr bool operator==(const Darray& lhs, const Darray& rhs) {
        if (lhs.size() != rhs.size()) {
            return false;
        }
        return std::equal(lhs.begin(), lhs.end(), rhs.begin());
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

    // Add an element to the end of the array
    constexpr void push_back(const T& value) {
        if (m_size + 1 > m_capacity) {
            m_capacity = std::max(1u, static_cast<unsigned int>(m_capacity * 2));
            auto new_data = std::make_unique<T[]>(m_capacity);
            std::copy(m_data.get(), m_data.get() + m_size, new_data.get());

            m_data.release();
            m_data = std::move(new_data);
        }

        m_data[m_size++] = value;
    }

    // Add an element to the front of the array
    constexpr void push_front(const T& value) {
        if (m_size + 1 > m_capacity) {
            m_capacity = std::max(1u, static_cast<unsigned int>(m_capacity * 2));
            auto new_data = std::make_unique<T[]>(m_capacity);

            // Move all elements one position to the right
            for (std::size_t i = m_size; i > 0; --i) {
                new_data[i] = std::move(m_data[i - 1]);
            }

            // Add the new value to the beginning of the new data
            new_data[0] = value;
            m_data.release();
            m_data = std::move(new_data);
        }
        else {
            // Move all elements one position to the right
            for (std::size_t i = m_size; i > 0; --i) {
                m_data[i] = std::move(m_data[i - 1]);
            }

            // Add the new value to the beginning of the data
            m_data[0] = value;
        }
        ++m_size;
    }

    // remove the last element from the array
    constexpr void pop_back() {
        // If array is empty, throw an error
        if (empty()) {
            throw std::logic_error("Array is empty");
        }

        --m_size;
        auto new_data = std::make_unique<T[]>(m_size);

        // Copy all elements except the last one from the original array to the new array
        std::copy(m_data.get(), m_data.get() + std::min(m_size, m_size), new_data.get());
        m_data.release();
        m_data = std::move(new_data);
    }

    // remove the first element from the array
    constexpr void pop_front() {
        if (empty()) {
            throw std::out_of_range("Array is empty");
        }

        --m_size;
        auto new_data = std::make_unique<T[]>(m_size);

        for (std::size_t i = 0; i < m_size; ++i) {
            new_data[i] = std::move(m_data[i + 1]);
        }

        m_data.release();
        m_data = std::move(new_data);
    }

    // remove element at index in the array
    // - index: the index in array to remove
    constexpr void remove(std::size_t index) {
        if (index >= m_size) {
            throw std::out_of_range("Index out of range");
        }

        // Shift all elements after the removed element to the left
        for (std::size_t i = index; i < m_size - 1; ++i) {
            m_data[i] = std::move(m_data[i + 1]);
        }

        --m_size;
    }

    // Insert an element at index of the array (Reallocation if capacity increases)
    // - index: the index to insert the element at
    // - value: the value to insert
    void insert(std::size_t index, const T& value) {
        if (index > m_size) {
            throw std::out_of_range("Index out of range");
        }

        if (m_size + 1 > m_capacity) {
            m_capacity = std::max(1u, static_cast<unsigned int>(m_capacity * 2));
            auto new_data = std::make_unique<T[]>(m_capacity);

            std::copy(m_data.get(), m_data.get() + index, new_data.get());
            new_data[index] = value;

            // Copy the elements after the insertion point
            std::copy(m_data.get() + index, m_data.get() + m_size, new_data.get() + index + 1);
            m_data.release();
            m_data = std::move(new_data);
        }
        else {
            auto new_data = std::make_unique<T[]>(m_size + 1);

            std::copy(m_data.get(), m_data.get() + index, new_data.get());
            new_data[index] = value;

            // Copy the elements after the insertion point
            std::copy(m_data.get() + index, m_data.get() + m_size, new_data.get() + index + 1);
            m_data.release();
            m_data = std::move(new_data);
        }

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
                std::copy(m_data.get(), m_data.get() + m_size, new_data.get());
                m_data.release();
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