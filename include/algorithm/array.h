#include <algorithm>
#include <memory>
#include <ranges>
#include <stdexcept>
#include <span>

template <typename T>
class Array {
public:
    using iterator = T*;
    using const_iterator = const T*;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;
    using span = std::span<T>;
    using const_span = std::span<const T>;

    Array() : Array(0) {}
    explicit Array(std::size_t size) : size_(size), data_(std::make_unique<T[]>(size_)) {}
    Array(std::initializer_list<T> init) : Array(init.size()) { std::ranges::copy(init, data_.get()); }
    Array(span init) : Array(init.size()) { std::ranges::copy(init, data_.get()); }
    Array(const Array& other) : Array(other.size_) { 
        std::copy(other.data_.get(), other.data_.get() + other.size_, data_.get());
        // std::ranges::copy(other.data_, data_.get()); commented out until it works with cpp 20
    }

    Array(Array&& other) : size_(other.size_), data_(std::move(other.data_)) { other.size_ = 0; }

    // Overloading the assignment operator to copy data from another Array object.
    Array& operator=(const Array& other) noexcept {
        if (this != &other) {
            size_ = other.size_;
            data_ = std::make_unique<T[]>(size_);
            std::ranges::copy(other.data_, data_.get());
        }

        return *this;
    }

    // Overloading the assignment operator to handle move semantics
    Array& operator=(Array&& other) noexcept {
        size_ = other.size_;
        data_ = std::move(other.data_);
        other.size_ = 0;

        return *this;
    }

    // Returns a reference to the element stored at the specified index in the array.
    T& operator[](std::size_t index) { return data_[index]; }

    // Returns a constant reference to the element stored at the specified index in the array.
    const T& operator[](std::size_t index) const { return data_[index]; }

    iterator begin() { return data_.get(); }
    const_iterator begin() const { return data_.get(); }
    const_iterator cbegin() const { return begin(); }
    iterator end() { return data_.get() + size_; }
    const_iterator end() const { return data_.get() + size_; }
    const_iterator cend() const { return end(); }
    reverse_iterator rbegin() { return reverse_iterator(end()); }
    const_reverse_iterator rbegin() const { return const_reverse_iterator(end()); }
    const_reverse_iterator crbegin() const { return rbegin(); }
    reverse_iterator rend() { return reverse_iterator(begin()); }
    const_reverse_iterator rend() const { return const_reverse_iterator(begin()); }
    const_reverse_iterator crend() const { return rend(); }

    // Returns a non-const span object that provides pointer access to the underlying stored data.
    span data() { return { data_.get(), size_ }; }

    // Returns a const span object that provides read-only pointer access to the underlying stored data.
    const_span data() const { return { data_.get(), size_ }; }

    // Returns size of array
    std::size_t size() const { return size_; }

    // Checks whether array is empty or not
    bool empty() const { return size_ == 0; }

    // Adds an element to the end of the array
    void push_back(const T& value) {
        // Check if the new size will be greater than the current size
        if (auto new_size = size_ + 1; new_size > size_) {
            auto new_data = std::make_unique<T[]>(new_size);

            // Copy the elements from the current data to the new data
            std::ranges::copy_n(data_.get(), size_, new_data.get());
            // Add the new value to the end of the new data
            new_data[size_] = value;
            data_ = std::move(new_data);
            size_ = new_size;
        }
        else {
            throw std::out_of_range("Array size overflow");
        }
    }

    // Adds an element to the end of the array
    void push_back(T&& value) {
       // Check if the new size will be greater than the current size
        if (auto new_size = size_ + 1; new_size > size_) {
            auto new_data = std::make_unique<T[]>(new_size);

            // Copy the elements from the current data to the new data
            std::ranges::copy_n(data_.get(), size_, new_data.get()); 
            // Add the new value to the end of the new data
            new_data[size_] = value;
            data_ = std::move(new_data);
            size_ = new_size;
        }
        else {
            throw std::out_of_range("Array size overflow");
        }
    }

    // remove the last element from the array
    void pop_back() {
        // If array is empty, throw an error
        if (empty()) {
            throw std::logic_error("Array is empty");
        }

        std::size_t new_size = size_ -1;
        auto new_data = std::make_unique<T[]>(new_size);

        // Copy all elements except the last one from the original array to the new array
        std::ranges::copy_n(data_.get(), std::min(size_, new_size), new_data.get());
        data_ = std::move(new_data);
        size_ = new_size;
    }

    // Inserts an element at a specified index of the array (Reallocation if size increases)
    // - index: the index to insert the element at
    // - value: the value to insert
    void insert(std::size_t index, const T& value) {
        if (index > size_) {
            throw std::out_of_range("Index out of range");
        }

        // Allocate more memory
        auto new_data = std::make_unique<T[]>(size_ + 1);

        std::copy(data_.get(), data_.get() + index, new_data.get());
        new_data[index] = value;

        // Copy the elements after the insertion point
        std::copy(data_.get() + index, data_.get() + size_, new_data.get() + index + 1);
        data_ = std::move(new_data);
        ++size_;
    }

private:
    std::size_t size_;
    std::unique_ptr<T[]> data_;
};