#pragma once
#include <algorithm>
#include <memory>
#include <ranges>
#include <stdexcept>
#include <span>
#include <cstddef>
#include <cstring>
#include "../allocators/simpleAllocator.h"

// Dynamic-sized array which increases size when at capacity
template <typename T, typename Alloc = SimpleAllocator<T>>
class DynamicArray {
public:
    using iterator = T*;
    using const_iterator = const T*;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;
    using span = std::span<T>;
    using const_span = std::span<const T>;

    constexpr iterator begin() noexcept { return m_data.get(); }
    constexpr const_iterator begin() const noexcept { return m_data.get(); }
    constexpr const_iterator cbegin() const noexcept { return begin(); }
    constexpr iterator end() noexcept { return m_data.get() + m_size; }
    constexpr const_iterator end() const noexcept { return m_data.get() + m_size; }
    constexpr const_iterator cend() const noexcept { return end(); }
    constexpr reverse_iterator rbegin() noexcept { return reverse_iterator(end()); }
    constexpr const_reverse_iterator rbegin() const noexcept { return const_reverse_iterator(end()); }
    constexpr const_reverse_iterator crbegin() const noexcept { return rbegin(); }
    constexpr reverse_iterator rend() noexcept { return reverse_iterator(begin()); }
    constexpr const_reverse_iterator rend() const noexcept { return const_reverse_iterator(begin()); }
    constexpr const_reverse_iterator crend() const noexcept { return rend(); }

    constexpr DynamicArray()
        : m_size(0),
        m_capacity(0),
        m_original_capacity(0),
        m_data(nullptr, ArrayDeleter<T, Alloc>()),
        m_allocator(Alloc()) {}

    constexpr explicit DynamicArray(std::size_t size, const Alloc& alloc = Alloc())
        : m_size(size),
        m_capacity(size),
        m_original_capacity(0),
        m_data(alloc.allocate(size), ArrayDeleter<T, Alloc>(alloc, size, m_capacity)),
        m_allocator(alloc) {
        for (size_t i = 0; i < m_size; ++i) {
            std::allocator_traits<Alloc>::construct(const_cast<Alloc&>(m_allocator), m_data.get() + i);
        }
    }

    constexpr explicit DynamicArray(std::span<const T> values, const Alloc& alloc = Alloc())
        : m_size(values.size()),
        m_capacity(values.size()),
        m_original_capacity(0),
        m_data(alloc.allocate(values.size()), ArrayDeleter<T, Alloc>(alloc, m_size, m_capacity)),
        m_allocator(alloc) {
        for (size_t i = 0; i < m_size; ++i) {
            std::allocator_traits<Alloc>::construct(const_cast<Alloc&>(m_allocator), m_data.get() + i, values[i]);
        }
    }

    DynamicArray(std::initializer_list<T> values, Alloc alloc = Alloc())
        : m_size(values.size()),
        m_capacity(values.size()),
        m_original_capacity(0),
        m_data(alloc.allocate(values.size()), ArrayDeleter<T, Alloc>(alloc, m_size, m_capacity)),
        m_allocator(alloc) {
        size_t i = 0;
        for (const auto& value : values) {
            alloc.construct(m_data.get() + i, value);
            ++i;
        }
    }

    constexpr DynamicArray(const DynamicArray& other)
        : m_size(other.m_size),
        m_capacity(other.m_size),
        m_original_capacity(other.m_original_capacity),
        m_data(m_allocator.allocate(other.size()), ArrayDeleter<T, Alloc>(m_allocator, m_size, m_capacity)),
        m_allocator(Alloc()) {
        std::uninitialized_copy(other.m_data.get(), other.m_data.get() + other.m_size, m_data.get());
    }

    constexpr DynamicArray(DynamicArray&& other)
        : m_size(other.m_size),
        m_capacity(other.m_capacity),
        m_original_capacity(other.m_original_capacity),
        m_data(std::move(other.m_data)),
        m_allocator(Alloc()) {
        other.m_size = 0;
        other.m_capacity = 0;
    }

    // Overloading the copy assignment operator to copy data from another DynamicArray object.
    constexpr DynamicArray& operator=(const DynamicArray& other) noexcept {
        if (this != &other) {
            if (m_capacity < other.m_size) {
                auto new_data = std::allocator_traits<Alloc>::allocate(m_allocator, other.m_size());

                if constexpr (!std::is_trivially_copyable_v<T>) {
                    std::uninitialized_copy(other.m_data.get(), other.m_data.get() + other.m_size, new_data);
                }
                else {
                    std::memcpy(new_data, other.m_data.get(), other.size() * sizeof(T));
                }

                std::destroy_n(m_data.get(), m_size);
                std::allocator_traits<Alloc>::deallocate(m_allocator, m_data.release(), m_capacity);

                m_data.reset(new_data);
                m_capacity = other.m_size;
                m_original_capacity = other.m_original_capacity;
                m_size = other.m_size;
            }
            else {
                std::copy(other.m_data.get(), other.m_data.get() + other.m_size, m_data.get());

                if (m_size > other.m_size) {
                    std::destroy(m_data.get() + other.m_size, m_data.get() + m_size);
                }

                m_size = other.m_size;
            }
        }
        return *this;
    }

    // Overloading the assignment operator to handle move semantics
    constexpr DynamicArray& operator=(DynamicArray&& other) noexcept {
        m_size = other.m_size;
        m_capacity = other.m_capacity;
        m_original_capacity = other.m_original_capacity;
        m_data = std::move(other.m_data);
        other.m_size = 0;

        return *this;
    }

    constexpr bool operator==(const DynamicArray& rhs) const noexcept {
        if (this->size() != rhs.size()) {
            return false;
        }
        return std::equal(this->begin(), this->end(), rhs.begin());
    }

    // Returns a reference to the element stored at the specified index in the array.
    constexpr T& operator[](std::size_t index) noexcept { return m_data[index]; }

    // Returns a constant reference to the element stored at the specified index in the array.
    constexpr const T& operator[](std::size_t index) const noexcept { return m_data[index]; }

    // Returns a non-const span object that provides pointer access to the underlying stored data.
    constexpr span data() noexcept { return { m_data.get(), m_size }; }

    // Returns a const span object that provides read-only pointer access to the underlying stored data.
    constexpr const_span data() const noexcept { return { m_data.get(), m_size }; }

    // Returns size of array
    constexpr std::size_t size() const noexcept { return m_size; }

    // Returns capacity of array
    constexpr std::size_t capacity() const noexcept { return m_capacity; }

    // Checks whether array is empty or not
    constexpr bool empty() const noexcept { return m_size == 0; }

    // Add an element to the end of the array
    void push_back(const T& value) {
        if (m_size == m_capacity) {
            // Allocate new memory
            std::size_t new_capacity = m_capacity == 0 ? 1 : m_capacity * 2;
            T* new_data = std::allocator_traits<Alloc>::allocate(m_allocator, new_capacity);

            // Copy elements to new memory
            std::uninitialized_copy_n(m_data.get(), m_size, new_data);

            // Destroy old elements and deallocate old memory
            std::unique_ptr<T[], ArrayDeleter<T, Alloc>> old_data(m_data.release(), ArrayDeleter<T, Alloc>(m_allocator, m_size, m_capacity));
            m_capacity = new_capacity;

            // Create a new unique_ptr to hold the new memory and transfer ownership
            std::unique_ptr<T[], ArrayDeleter<T, Alloc>> new_data_ptr(new_data, ArrayDeleter<T, Alloc>(m_allocator, m_size, new_capacity));
            std::swap(m_data, new_data_ptr);
        }

        // Construct new element
        std::allocator_traits<Alloc>::construct(m_allocator, m_data.get() + m_size, value);
        ++m_size;
    }

    // Add an element to the front of the array
    constexpr void push_front(const T& value) {
        if (m_size + 1 > m_capacity) {
            m_original_capacity = m_capacity; // update m_original_capacity
            m_capacity = std::max(1u, static_cast<unsigned int>(m_capacity * 2));
            auto new_data = std::allocator_traits<Alloc>::allocate(m_allocator, m_capacity);

            // Move all elements one position to the right
            std::uninitialized_move(m_data.get(), m_data.get() + m_size, std::addressof(*(new_data + 1)));

            // Construct the new value at the beginning of the new data
            std::allocator_traits<Alloc>::construct(m_allocator, new_data, value);

            // Destroy the old elements
            for (size_t i = 0; i < m_size; ++i) {
                std::allocator_traits<Alloc>::destroy(m_allocator, m_data.get() + i);
            }

            // Deallocate the old memory
            std::allocator_traits<Alloc>::deallocate(m_allocator, m_data.release(), m_capacity);

            m_data.reset(new_data);
        }
        else {
            // Move all elements one position to the right
            std::uninitialized_move(m_data.get(), m_data.get() + m_size, m_data.get() + 1);

            // Construct the new value at the beginning of the data
            std::allocator_traits<Alloc>::construct(m_allocator, m_data.get(), value);
        }

        ++m_size;
    }

    // remove the last element from the array
    constexpr void pop_back() {
        // If array is empty, throw an error
        if (empty()) {
            throw std::logic_error("Array is empty");
        }

        // Destroy the last element in the array
        std::allocator_traits<Alloc>::destroy(m_allocator, m_data.get() + m_size - 1);

        // Decrement the size of the array
        --m_size;

        // Allocate memory for the new array
        auto new_data = std::allocator_traits<Alloc>::allocate(m_allocator, m_size);

        // Copy all elements except the last one from the old array to the new array
        std::uninitialized_copy(m_data.get(), m_data.get() + m_size, new_data);

        // Destroy the old elements
        for (size_t i = 0; i < m_size + 1; ++i) {
            std::allocator_traits<Alloc>::destroy(m_allocator, m_data.get() + i);
        }

        // Deallocate the memory used by the old array
        std::allocator_traits<Alloc>::deallocate(m_allocator, m_data.release(), m_capacity);

        // Point to the new array
        m_data.reset(new_data);
    }

    // remove the first element from the array
    constexpr void pop_front() {
        if (empty()) {
            throw std::logic_error("Array is empty");
        }

        // Destroy the first element in the array
        std::allocator_traits<Alloc>::destroy(m_allocator, m_data.get());

        // Decrement the size of the array
        --m_size;

        // Allocate memory for the new array
        auto new_data = std::allocator_traits<Alloc>::allocate(m_allocator, m_size);

        // Move the existing elements to the left in the new memory
        std::uninitialized_move(m_data.get() + 1, m_data.get() + m_size + 1, new_data);

        // Destroy the old elements
        for (size_t i = 0; i < m_size + 1; ++i) {
            std::allocator_traits<Alloc>::destroy(m_allocator, m_data.get() + i);
        }

        // Deallocate the memory used by the old array
        std::allocator_traits<Alloc>::deallocate(m_allocator, m_data.release(), m_capacity);

        // Point to the new array
        m_data.reset(new_data);
    }

    // remove element at index in the array
    // - index: the index in array to remove
    constexpr void remove(std::size_t index) {
        if (index >= m_size) {
            throw std::out_of_range("Index out of range");
        }

        if (m_size == 0) {
            return;
        }

        // Shift all elements after the removed element to the left (Does not matter if index is at end of array)
        std::move_backward(m_data.get() + index + 1, m_data.get() + m_size, m_data.get() + m_size - 1);

        --m_size;
    }

    // Insert an element at index of the array (Reallocation if capacity increases)
    // - index: the index to insert the element at
    // - value: the value to insert
    constexpr void insert(std::size_t index, const T& value) {
        if (index > m_size) {
            throw std::out_of_range("Index out of range");
        }

        if (m_size == m_capacity) {
            m_original_capacity = m_capacity; // update m_original_capacity
            m_capacity = m_capacity == 0 ? 1 : m_capacity * 2;
        }

        // Allocate memory for the new array
        auto new_data = std::allocator_traits<Alloc>::allocate(m_allocator, m_capacity);

        // Copy the elements before the insertion point to the new array
        std::uninitialized_copy(m_data.get(), m_data.get() + index, new_data);

        // Construct the new element in the new array
        std::allocator_traits<Alloc>::construct(m_allocator, new_data + index, value);

        // Copy the elements after the insertion point to the new array
        std::uninitialized_copy(m_data.get() + index, m_data.get() + m_size, new_data + index + 1);

        // Destroy the old elements
        for (size_t i = 0; i < m_size; ++i) {
            std::allocator_traits<Alloc>::destroy(m_allocator, m_data.get() + i);
        }

        // Deallocate the memory used by the old array
        std::allocator_traits<Alloc>::deallocate(m_allocator, m_data.release(), m_original_capacity);

        // Point to the new array
        m_data.reset(new_data);
        m_capacity *= 2;
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
            auto new_data = std::allocator_traits<Alloc>::allocate(m_allocator, new_capacity);

            if (!new_data) {
                throw std::bad_alloc();
            }

            if (new_data != m_data.get()) {
                std::uninitialized_copy(m_data.get(), m_data.get() + m_size, new_data);
                std::allocator_traits<Alloc>::deallocate(m_allocator, m_data.release(), m_capacity);
                m_data.release();

                auto new_data_ptr = std::unique_ptr<T[], ArrayDeleter<T, Alloc>>(new_data, ArrayDeleter<T, Alloc>(m_allocator, m_size, m_capacity));
                m_data = std::move(new_data_ptr);
                m_capacity = new_capacity;
            }
        }
    }

private:
    template <typename T1, typename Alloc1>
    class ArrayDeleter {
    public:
        ArrayDeleter()
            : m_allocator(Alloc1()), m_size(0), m_capacity(0), m_num_destroyed(0) {}

        explicit ArrayDeleter(Alloc1& alloc, std::size_t size, std::size_t capacity)
            : m_allocator(alloc), m_size(size), m_capacity(capacity), m_num_destroyed(0) {}

        explicit ArrayDeleter(const Alloc& alloc, std::size_t size, std::size_t capacity)
            : m_allocator(alloc), m_size(size), m_capacity(capacity), m_num_destroyed(0) {}

        void operator()(T1* data) {
            if (data) {
                for (size_t i = 0; i < m_size; ++i) {
                    std::allocator_traits<Alloc1>::destroy(m_allocator, data + i);
                }

                if (m_num_destroyed == m_size - 1) {
                    std::allocator_traits<Alloc1>::deallocate(m_allocator, data, m_capacity);
                }

                ++m_num_destroyed;
            }
        }

        // Move constructor and move assignment operator
        ArrayDeleter(ArrayDeleter&& other) noexcept
            : m_allocator(other.m_allocator), 
            m_size(other.m_size), 
            m_capacity(other.m_capacity), 
            m_num_destroyed(other.m_num_destroyed) {}

        ArrayDeleter& operator=(ArrayDeleter&& other) noexcept {
            m_allocator = other.m_allocator;
            m_size = other.m_size;
            m_capacity = other.m_capacity;
            m_num_destroyed = other.m_num_destroyed;
            return *this;
        }

    private:
        Alloc1 m_allocator;
        std::size_t m_size;
        std::size_t m_capacity;
        std::size_t m_num_destroyed;
    };

    std::size_t m_size;
    std::size_t m_capacity;
    std::size_t m_original_capacity;
    std::unique_ptr<T[], ArrayDeleter<T, Alloc>> m_data;
    Alloc m_allocator;
};