#pragma once
#include <algorithm>
#include <memory>
#include <ranges>
#include <stdexcept>
#include <span>
#include <cstddef>
#include <cstring>
#include <compare>
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

    using value_type = T;
    using size_type = std::size_t;
    using reference = T&;
    using const_reference = const T&;

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
        m_data(other.m_allocator.allocate(other.size()), ArrayDeleter<T, Alloc>(m_allocator, m_size, m_capacity)),
        m_allocator(other.m_allocator) {
        std::uninitialized_copy(other.m_data.get(), other.m_data.get() + other.m_size, m_data.get());
    }

    constexpr DynamicArray(DynamicArray&& other)
        : m_size(other.m_size),
        m_capacity(other.m_capacity),
        m_original_capacity(other.m_original_capacity),
        m_data(std::move(other.m_data)),
        m_allocator(other.m_allocator) {
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

    constexpr std::strong_ordering operator<=>(const DynamicArray<T, Alloc>& rhs) const {
        if (size() < rhs.size()) {
            return std::strong_ordering::less;
        } else if (size() > rhs.size()) {
            return std::strong_ordering::greater;
        }

        auto mismatch = std::mismatch(begin(), end(), rhs.begin());
        if (mismatch.first == end()) {
            return std::strong_ordering::equal;
        }
        return *(mismatch.first) <=> *(mismatch.second);
    }

    // Returns a reference to the element stored at the specified index in the array.
    constexpr T& operator[](std::size_t index) noexcept { 
        return m_data[index]; 
    }

    // Returns a constant reference to the element stored at the specified index in the array.
    constexpr const T& operator[](std::size_t index) const noexcept { 
        return m_data[index]; 
    }

    // Returns a non-const span object that provides pointer access to the underlying stored data.
    constexpr span data() noexcept { 
        return { m_data.get(), m_size }; 
    }

    // Returns a const span object that provides read-only pointer access to the underlying stored data.
    constexpr const_span data() const noexcept { 
        return { m_data.get(), m_size }; 
    }

    // Return number of elements in the array
    constexpr std::size_t size() const noexcept { 
        return m_size; 
    }

    // Returns maximum number of elements allowed due to system restrictions
    constexpr std::size_t max_size() const noexcept { 
        return std::allocator_traits<Alloc>::max_size(m_allocator); 
    }

    // Returns capacity of array
    constexpr std::size_t capacity() const noexcept { 
        return m_capacity; 
    }

    // Checks whether array is empty or not
    [[nodiscard]] constexpr bool empty() const noexcept { 
        return m_size == 0; 
    }

    // Returns m_allocator associated with the dynamic array
    constexpr Alloc get_allocator() const noexcept { 
        return m_allocator; 
    }

    constexpr void assign(std::size_t count, const T& value) {
        if (count > m_capacity) {
            std::allocator_traits<Alloc>::destroy(m_allocator, m_data.get(), m_data.get() + m_size);
            std::allocator_traits<Alloc>::deallocate(m_allocator, m_data.release(), m_capacity);

            m_data.reset(std::allocator_traits<Alloc>::allocate(m_allocator, count));
            m_capacity = count;
        }

        std::fill_n(m_data.get(), count, value);
        m_size = count;
    }

    constexpr void assign(std::initializer_list<T> ilist) {
        assign(ilist.begin(), ilist.end());
    }

    template <class InputIt>
    constexpr void assign(InputIt first, InputIt last) {
        auto count = std::distance(first, last);
        if (count > m_capacity) {
            std::allocator_traits<Alloc>::destroy(m_allocator, m_data.get(), m_data.get() + m_size);
            std::allocator_traits<Alloc>::deallocate(m_allocator, m_data.release(), m_capacity);

            m_data.reset(std::allocator_traits<Alloc>::allocate(m_allocator, count));
            m_capacity = count;
        }

        std::uninitialized_copy(first, last, m_data.get());
        std::allocator_traits<Alloc>::destroy(m_allocator, m_data.get() + m_size);
        m_size = count;
    }

    // Access element by index, throws if not within the bounds of the array.
    // - index: the index in array to access
    constexpr T& at(std::size_t index) {
        if (index >= m_size) {
            throw std::out_of_range("Index out of range");
        }

        return m_data[index];
    }

    // Access const element by index, throws if not within the bounds of the array.
    // - index: the index in array to access
    constexpr T& at(std::size_t index) const {
        if (index >= m_size) {
            throw std::out_of_range("Index out of range");
        }

        return m_data[index];
    }

    // Returns a reference to the first element
    constexpr T& front() { 
        if (empty()) {
            throw std::out_of_range("Array is empty");
        }
        return m_data[0]; 
    }

    // Returns a const reference to the first element
    constexpr T& front() const {
        if (empty()) {
            throw std::out_of_range("Array is empty");
        }
        return m_data[0]; 
    }

    // Returns a reference to the last element
    constexpr T& back() {
        if (empty()) {
            throw std::out_of_range("Array is empty");
        }
        return m_data[m_size - 1];
    }

    // Returns a const reference to the last element
    constexpr T& back() const {
        if (empty()) {
            throw std::out_of_range("Array is empty");
        }
        return m_data[m_size - 1];
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
                std::allocator_traits<Alloc>::deallocate(m_allocator, m_data.get(), m_capacity);

                auto new_data_ptr = std::unique_ptr<T[], 
                    ArrayDeleter<T, Alloc>>(new_data, ArrayDeleter<T, Alloc>(m_allocator, m_size, m_capacity));
                m_data = std::move(new_data_ptr);
                m_capacity = new_capacity;
            }
        }
    }

    // Reduce memory usage by freeing unused allocated memory  
    constexpr void shrink_to_fit() {
        if (m_capacity > m_size) {
            auto new_data = std::allocator_traits<Alloc>::allocate(m_allocator, m_size);

            if constexpr (!std::is_trivially_copyable_v<T>) {
                std::uninitialized_copy(m_data.get(), m_data.get() + m_size, new_data);
            }
            else {
                std::memcpy(new_data, m_data.get(), m_size * sizeof(T));
            }

            std::destroy_n(m_data.get(), m_size);
            std::allocator_traits<Alloc>::deallocate(m_allocator, m_data.release(), m_capacity);

            m_data.reset(new_data);
            m_capacity = m_size;
        }
    }

    // Erase all elements
    constexpr void clear() noexcept {
        std::allocator_traits<Alloc>::destroy(m_allocator, m_data.get(), m_data.get() + m_size);
        std::allocator_traits<Alloc>::deallocate(m_allocator, m_data.release(), m_capacity);
        m_data.reset(nullptr);
        m_capacity = 0;
        m_size = 0;
    }

    constexpr iterator insert(const_iterator index, const T& value) {
        return emplace(index, value);
    }

    constexpr iterator insert(const_iterator index, T&& value) {
        return emplace(index, std::move(value));
    }

    constexpr iterator insert(const_iterator index, std::size_t count, const T& value) {
        if (index < cbegin() || index > cend()) {
            throw std::out_of_range("Invalid index");
        }

        auto i = begin() + std::distance(cbegin(), index);

        if (m_size + count > m_capacity) {
            std::size_t new_capacity = m_capacity == 0 ? 1 : m_capacity * 2;
            T* new_data = std::allocator_traits<Alloc>::allocate(m_allocator, new_capacity);

            std::uninitialized_move_n(m_data.get(), i - m_data.get(), new_data);
            std::uninitialized_fill_n(new_data + (i - m_data.get()), count, value);
            std::uninitialized_move_n(i, m_size - (i - m_data.get()), new_data + (i - m_data.get()) + count);

            auto first = i;
            auto last = i + count;

            for (; first != last; ++first) {
                std::allocator_traits<Alloc>::destroy(m_allocator, std::addressof(*first));
            }

            std::allocator_traits<Alloc>::deallocate(m_allocator, m_data.release(), m_capacity);

            m_data.reset(new_data);
            m_capacity = new_capacity;
            m_size += count;
        }
        else {
            std::uninitialized_move_n(i, m_size - (i - m_data.get()), i + count);
            std::uninitialized_fill_n(i, count, value);

            m_size += count;
        }

        return i;
    }

    constexpr iterator insert(const_iterator index, std::initializer_list<T> ilist) {
        return insert(index, ilist.begin(), ilist.end());
    }

    // TODO: Fix code
    /*
    template <class InputIt>
    constexpr iterator insert(const_iterator index, InputIt first, InputIt last) {
        if (index < cbegin() || index > cend()) {
            throw std::out_of_range("Invalid index");
        }

        auto count = std::distance(first, last);
        auto i = begin() + std::distance(cbegin(), index);

        if (m_size + count > m_capacity) {
            std::size_t new_capacity = m_capacity == 0 ? 1 : m_capacity * 2;
            T* new_data = std::allocator_traits<Alloc>::allocate(m_allocator, new_capacity);

            std::uninitialized_move_n(m_data.get(), i - m_data.get(), new_data);
            std::uninitialized_copy(first, last, new_data + (i - m_data.get()));
            std::uninitialized_move_n(i, m_size - (i - m_data.get()), new_data + (i - m_data.get()) + count);

            for (; first != last; ++first) {
                std::allocator_traits<Alloc>::destroy(m_allocator, std::addressof(*first));
            }

            std::allocator_traits<Alloc>::deallocate(m_allocator, m_data.release(), m_capacity);

            m_data.reset(new_data);
            m_capacity = new_capacity;
            m_size += count;
        }
        else {
            std::uninitialized_move_n(i, m_size - (i - m_data.get()), i + count);
            std::uninitialized_copy(first, last, i);

            std::allocator_traits<Alloc>::destroy(m_allocator, m_data.get() + m_size);
            m_size += count;
        }

        return i;
    }
    */

    // Appends a new element before index in the array.
    template <class... Args>
    constexpr iterator emplace(const_iterator index, Args&&... args) {
        // Convert the iterator to a pointer offset
        std::size_t index_offset = index - cbegin();

        if (index_offset > m_size) {
            throw std::out_of_range("Invalid index");
        }

        if (m_size == m_capacity) {
            m_original_capacity = m_capacity; // update m_original_capacity
            m_capacity = m_capacity == 0 ? 1 : m_capacity * 2;
        }

        // Allocate memory for the new array
        auto new_data = std::allocator_traits<Alloc>::allocate(m_allocator, m_capacity);

        // Copy the elements before the insertion point to the new array
        std::uninitialized_copy(m_data.get(), m_data.get() + index_offset, new_data);

        // Construct the new element in the new array
        std::allocator_traits<Alloc>::construct(m_allocator, new_data + index_offset, std::forward<Args>(args)...);

        // Copy the elements after the insertion point to the new array
        std::uninitialized_copy(m_data.get() + index_offset, m_data.get() + m_size, new_data + index_offset + 1);

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

        // Return an iterator pointing to the new element
        return m_data.get() + index_offset;     
    }

    // Appends a new element to the end of the array.
    template <class... Args>
    constexpr T& emplace_back(Args&&... args) {
        if (m_size == m_capacity) {
            reserve(m_capacity == 0 ? 1 : m_capacity * 2);
        }

        std::allocator_traits<Alloc>::construct(m_allocator, end(), std::forward<Args>(args)...);
        ++m_size;

        return back();
    }

    // Remove element at index
    constexpr iterator erase(const_iterator index) {
        if (index < cbegin() || index >= cend()) {
            throw std::out_of_range("Invalid index");
        }

        auto i = begin() + (index - cbegin());
        //auto i = begin() + std::distance(cbegin(), index);
        std::allocator_traits<Alloc>::destroy(m_allocator, i);

        std::move(i + 1, end(), i);

        --m_size;

        if (m_size < m_capacity / 2) {
            shrink_to_fit();
        }

        return i;
    }

    // Remove elements in range [first, last]
    constexpr iterator erase(const_iterator first, const_iterator last) {
        if (first < cbegin() || last > cend() || first > last) {
            throw std::out_of_range("Invalid range");
        }

        auto i = begin() + std::distance(cbegin(), first);
        auto j = begin() + std::distance(cbegin(), last);

        std::allocator_traits<Alloc>::destroy(m_allocator, i, j);

        std::move(j, end(), i);

        m_size -= std::distance(i, j);

        if (m_size < m_capacity / 2) {
            shrink_to_fit();
        }

        return i;
    }

    // Resizes array to contain 'count' of elements of type 'value'
    constexpr void resize(std::size_t count, const T& value) {
        if (count < m_size) {
            std::allocator_traits<Alloc>::destroy(m_allocator, m_data.get() + count, m_data.get() + m_size);
            m_size = count;
        }
        else if (count > m_size && count <= m_capacity) {
            std::uninitialized_fill(m_data.get() + m_size, m_data.get() + count, value);
            m_size = count;
        }
        else {
            auto new_data = std::allocator_traits<Alloc>::allocate(m_allocator, count);
            std::uninitialized_fill(new_data, new_data + count, value);
            std::allocator_traits<Alloc>::destroy(m_allocator, m_data.get(), m_data.get() + m_size);
            std::allocator_traits<Alloc>::deallocate(m_allocator, m_data.release(), m_capacity);
            m_data.reset(new_data);
            m_size = count;
            m_capacity = count;
        }
    }

    // Exchanges the contents and capacity of the container with those of other
    constexpr void swap(DynamicArray& other) noexcept {
        std::swap(m_data, other.m_data);
        std::swap(m_size, other.m_size);
        std::swap(m_capacity, other.m_capacity);
        std::swap(m_original_capacity, other.m_original_capacity);
        std::swap(m_allocator, other.m_allocator);
    }

    // Add an element to the end of the array
    constexpr void push_back(const T& value) {
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

    constexpr void push_back(T&& value) {
        if (m_size == m_capacity) {
            // Allocate new memory
            std::size_t new_capacity = m_capacity == 0 ? 1 : m_capacity * 2;
            T* new_data = std::allocator_traits<Alloc>::allocate(m_allocator, new_capacity);

            // Move elements to new memory
            std::uninitialized_move_n(m_data.get(), m_size, new_data);

            // Destroy old elements and deallocate old memory
            std::unique_ptr<T[], ArrayDeleter<T, Alloc>> old_data(m_data.release(), ArrayDeleter<T, Alloc>(m_allocator, m_size, m_capacity));
            m_capacity = new_capacity;

            // Create a new unique_ptr to hold the new memory and transfer ownership
            std::unique_ptr<T[], ArrayDeleter<T, Alloc>> new_data_ptr(new_data, ArrayDeleter<T, Alloc>(m_allocator, m_size, new_capacity));
            std::swap(m_data, new_data_ptr);
        }

        // Construct new element
        std::allocator_traits<Alloc>::construct(m_allocator, m_data.get() + m_size, std::move(value));
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