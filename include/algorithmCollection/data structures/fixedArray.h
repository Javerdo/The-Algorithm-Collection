#pragma once
#include <algorithm>
#include <memory>
#include <ranges>
#include <stdexcept>
#include <cstddef>
#include <span>
#include "../allocators/simpleAllocator.h"

// Fixed-size array with memory safety
template <typename T, std::size_t S, typename Alloc = SimpleAllocator<T>>
class FixedArray {
public:
    using iterator = T*;
    using const_iterator = const T*;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;
    using span = std::span<T>;
    using const_span = std::span<const T>;

    constexpr FixedArray() {
        allocate_memory();
        std::fill(m_data.begin(), m_data.end(), T{});
    }

    constexpr FixedArray(std::initializer_list<T> values) : m_data(nullptr) {
        if (values.size() > S) {
            throw std::invalid_argument("Initializer list size is greater than array size");
        }
        
        std::copy(values.begin(), values.end(), m_data.begin());
    }

    constexpr FixedArray(const FixedArray& other) : m_data(nullptr) {
        allocate_memory();
        std::copy(other.cbegin(), other.cend(), m_data.begin());
    }

    ~FixedArray() {
        deallocate_memory();
    }

    constexpr FixedArray& operator=(const FixedArray& other) noexcept {
        if (this == &other) { return *this; }
        
        if (other.size() > S) {
            // Allocate a new array of the appropriate size
            FixedArray<T, other.size()> new_data;
            
            // Copy elements from the old and new arrays to the new array
            std::copy(other.cbegin(), other.cend(), new_data.begin());
            std::fill(new_data.begin() + S, new_data.end(), T{});
            
            // Deallocate the old array
            deallocate_memory();
            
            // Update the data pointer and size
            m_data = new_data;
        } else {
            // Copy elements from the old and new arrays to the current array
            std::copy(other.cbegin(), other.cend(), m_data.begin());
            std::fill(m_data.begin() + other.size(), m_data.end(), T{});
        }
        
        return *this;
    }

    // Returns a reference to the element stored at the specified index in the array.
    constexpr T& operator[](std::size_t index) noexcept { return m_data[index]; }

    // Returns a constant reference to the element stored at the specified index in the array.
    constexpr const T& operator[](std::size_t index) const noexcept { 
        if (index >= S) {
            throw std::out_of_range("Index out of range");
        }

        return m_data[index]; 
    }

    constexpr bool operator==(const FixedArray& lhs, const FixedArray& rhs) noexcept { 
        return std::equal(lhs.cbegin(), lhs.cend(), rhs.cbegin());
    }

    constexpr bool operator!=(const FixedArray& lhs, const FixedArray& rhs) noexcept { 
        return !(lhs == rhs); 
    }

    constexpr iterator begin() noexcept { return m_data.get(); }
    constexpr const_iterator begin() const noexcept { return m_data.get(); }
    constexpr const_iterator cbegin() const noexcept { return begin(); }
    constexpr iterator end() noexcept { return m_data.get() + S; }
    constexpr const_iterator end() const noexcept { return m_data.get() + S; }
    constexpr const_iterator cend() const noexcept { return end(); }
    constexpr reverse_iterator rbegin() noexcept { return reverse_iterator(end()); }
    constexpr const_reverse_iterator rbegin() const noexcept { return const_reverse_iterator(end()); }
    constexpr const_reverse_iterator crbegin() const noexcept { return rbegin(); }
    constexpr reverse_iterator rend() noexcept { return reverse_iterator(begin()); }
    constexpr const_reverse_iterator rend() const noexcept { return const_reverse_iterator(begin()); }
    constexpr const_reverse_iterator crend() const noexcept { return rend(); }

    // Returns a non-const span object that provides pointer access to the underlying stored data.
    constexpr span data() noexcept { return { m_data.get(), S }; }

    // Returns a const span object that provides read-only pointer access to the underlying stored data.
    constexpr const_span data() const noexcept { return { m_data.get(), S }; }

    // Returns size of array
    constexpr std::size_t size() const noexcept { return S; }

    // Checks whether array is empty or not
    constexpr bool empty() const noexcept { return S == 0; }

    // Returns a reference to first element in array
    constexpr T& front() {
        if (empty()) {
            throw std::logic_error("Array is empty");
        }

        return m_data[0];
    }

    // Returns a constant reference to first element in array
    constexpr T& front() const {
        if (empty()) {
            throw std::logic_error("Array is empty");
        }

        return m_data[0];
    }

    // Returns a reference to last element in array
    constexpr T& back() {
        if (empty()) {
            throw std::logic_error("Array is empty");
        }

        return m_data[S-1];
    }

    // Returns a constant reference to last element in array
    constexpr T& back() const {
        if (empty()) {
            throw std::logic_error("Array is empty");
        }

        return m_data[S-1];
    }

    // Access element by index, throws if not within the bounds of the array.
    constexpr T& at(std::size_t index) const {
        if (index >= S) {
            throw std::out_of_range("Index out of range");
        }

        return m_data[index];
    }

    // Fills array with a specified value
    void fill(const T& value) noexcept {
        std::fill(begin(), end(), value);
    }

    // Swaps data with target array
    void swap(FixedArray& other) noexcept {
        std::swap(m_data, other.m_data);
    }

private:
    std::unique_ptr<T[S]> m_data;

    void allocate_memory() {
        std::uninitialized_fill_n(m_data.get(), S, T{});
        m_data = m_allocator.allocate(S);
    }

    void deallocate_memory() {
        m_allocator.deallocate(m_data, S);
    }
};