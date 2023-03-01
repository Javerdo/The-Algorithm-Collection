#pragma once
#include <memory>
#include <stdexcept>
#include <compare>
#include <algorithm>
#include <functional>
#include <utility>
#include "simpleAllocator.h"

// Double linked list with custom memory allocation and safety
template <typename T, typename Alloc = SimpleAllocator<T>>
class DoubleLinkedList {
private:
    struct Node {
        T data;
        Node* next;
        Node* prev;

        Node(T value, Node* n = nullptr, Node* p = nullptr)
            : data(value), next(n), prev(p) {}
    };

public:
    using iterator = Node*;
    using const_iterator = const Node*;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    constexpr iterator begin() noexcept { return head.get(); }
    constexpr const_iterator begin() const noexcept { return head.get(); }
    constexpr const_iterator cbegin() const noexcept { return begin(); }
    constexpr iterator end() noexcept { return tail.get(); }
    constexpr const_iterator end() const noexcept { return tail.get(); }
    constexpr const_iterator cend() const noexcept { return end(); }
    constexpr reverse_iterator rbegin() noexcept { return reverse_iterator(end()); }
    constexpr const_reverse_iterator rbegin() const noexcept { return const_reverse_iterator(end()); }
    constexpr const_reverse_iterator crbegin() const noexcept { return rbegin(); }
    constexpr reverse_iterator rend() noexcept { return reverse_iterator(begin()); }
    constexpr const_reverse_iterator rend() const noexcept { return const_reverse_iterator(begin()); }
    constexpr const_reverse_iterator crend() const noexcept { return rend(); }

    using data_type = T;
    using node_type = std::unique_ptr<Node, std::function<void(Node*)>>;
    using allocator_type = typename std::allocator_traits<Alloc>::template rebind_alloc<Node>;
    using size_type = std::size_t;

    explicit DoubleLinkedList(const allocator_type& alloc = allocator_type())
        : m_allocator(alloc), head(nullptr), tail(nullptr), m_size(0) {}

    DoubleLinkedList(size_type size, const data_type& value, const allocator_type& alloc = allocator_type())
        : m_allocator(alloc), head(nullptr), tail(nullptr), m_size(0) {
        for (size_type i = 0; i < size; ++i) {
            push_back(value);
        }
    }

    explicit DoubleLinkedList(size_type size, const allocator_type& alloc = allocator_type())
        : m_allocator(alloc), head(nullptr), tail(nullptr), m_size(0) {}

    template <class InputIt>
    DoubleLinkedList(InputIt first, InputIt last, const allocator_type& alloc = allocator_type())
        : m_allocator(alloc), head(nullptr), tail(nullptr), m_size(0) {
        static_assert(std::is_same_v<std::decay_t<decltype(*first)>, T>, 
            "Iterator value type does not match linked list value type");
        while (first != last) {
            push_back(*first);
            ++first;
        }
    }

    DoubleLinkedList(const DoubleLinkedList& other)
        : DoubleLinkedList(other, other.get_allocator()) {}

    DoubleLinkedList(const DoubleLinkedList& other, const allocator_type& alloc)
        : m_allocator(alloc), head(nullptr), tail(nullptr), m_size(0) {
        for (const auto& value : other) {
            push_back(value);
        }
    }

    DoubleLinkedList(DoubleLinkedList&& other)
        : m_allocator(std::move(other.get_allocator())), head(std::move(other.head)),
        tail(std::move(other.tail)), m_size(other.m_size) {
        other.head = nullptr;
        other.tail = nullptr;
        other.m_size = 0;
    }

    DoubleLinkedList(DoubleLinkedList&& other, const allocator_type& alloc)
        : m_allocator(alloc), head(nullptr), tail(nullptr), m_size(0) {
        if (m_allocator == other.get_allocator()) {
            head = std::move(other.head);
            tail = std::move(other.tail);
            m_size = other.m_size;
            other.head = nullptr;
            other.tail = nullptr;
            other.m_size = 0;
        }
        else {
            for (const auto& value : other) {
                push_back(value);
            }
        }
    }

    DoubleLinkedList(std::initializer_list<data_type> init, const allocator_type& alloc = allocator_type())
        : m_allocator(alloc), head(nullptr), tail(nullptr), m_size(0) {
        static_assert(std::is_same_v<std::decay_t<decltype(*init.begin())>, data_type>,
            "Initializer list value type does not match linked list value type");

        if (init.size() == 0) {
            return;
        }

        for (const auto& value : init) {
            push_back(value);
        }
    }

    ~DoubleLinkedList() { clear(); }

    // Copy assignment
    DoubleLinkedList& operator=(const DoubleLinkedList& other) {
        if (this != &other) {
            clear();
            for (const auto& value : other) {
                push_back(value);
            }
        }

        return *this;
    }

    // Move assignment
    DoubleLinkedList& operator=(DoubleLinkedList&& other) noexcept {
        if (this != &other) {
            clear();
            m_allocator = std::move(other.m_allocator);
            head = std::move(other.head);
            tail = std::move(other.tail);
            m_size = other.m_size;

            other.head = nullptr;
            other.tail = nullptr;
            other.m_size = 0;
        }

        return *this;
    }

    // copy assignment (initializer list)
    DoubleLinkedList& operator=(std::initializer_list<data_type> ilist) {
        clear();
        for (const auto& value : ilist) {
            push_back(value);
        }

        return *this;
    }

    // Comparison operator
    std::strong_ordering operator<=>(const DoubleLinkedList<data_type, allocator_type>& other) {
        auto l_iter = begin(), l_end = end();
        auto r_iter = other.begin(), r_end = other.end();

        while (l_iter != l_end && r_iter != r_end) {
            if (l_iter->data < r_iter->data) {
                return std::strong_ordering::less;
            }
            else if (r_iter->data < l_iter->data) {
                return std::strong_ordering::greater;
            }
            ++l_iter;
            ++r_iter;
        }
        if (l_iter == l_end && r_iter == r_end) {
            return std::strong_ordering::equal;
        }
        else if (l_iter == l_end) {
            return std::strong_ordering::less;
        }
        else {
            return std::strong_ordering::greater;
        }
    }

    size_type size() const noexcept { return m_size; }
    [[nodiscard]] constexpr bool empty() const noexcept { return m_size == 0; }

    // Returns m_allocator associated with the linked list
    allocator_type& get_allocator() const noexcept { return m_allocator; }

    // Get first element in linked list
    node_type front() { return head; }
    constexpr node_type front() const { return head; }

    // Get last element in linked list
    node_type back() { return tail; }
    constexpr node_type back() const { return tail; }

    // Replace elements with copies of value
    void assign(size_type size, const data_type& value) {}

    // Replace elements with copies in the range (first, last)
    template <class InputIt>
    void assign(InputIt first, InputIt last) {}

    // Replace elements with copies from initializer list
    void assign(std::initializer_list<data_type> ilist) {}

    // Adds an element to the end of the linked list
    void push_back(const data_type& value) {
        node_type newNode(
            std::allocator_traits<allocator_type>::allocate(m_allocator, 1),
            [&](Node* n) {
                std::allocator_traits<allocator_type>::destroy(m_allocator, n);
                std::allocator_traits<allocator_type>::deallocate(m_allocator, n, 1);
            }
        );

        ++m_size;
    }

    void push_back(data_type&& value) {
        node_type newNode(
            std::allocator_traits<allocator_type>::allocate(m_allocator, 1),
            [&](Node* n) {
                std::allocator_traits<allocator_type>::destroy(m_allocator, n);
                std::allocator_traits<allocator_type>::deallocate(m_allocator, n, 1);
            }
        );

        ++m_size;
    }

    // Adds an element to the front of the linked list
    void push_front(const data_type& value) {}

    void push_front(data_type&& value) {}

    // Insert a single element at the specified index
    iterator insert(const_iterator index, const data_type& value) { 
        return iterator;
    }

    iterator insert(const_iterator index, data_type&& value) {
        return iterator;
    }

    // Insert multiple copies of an element at the specified index
    iterator insert(const_iterator index, size_type size, const data_type& value) {
        return iterator;
    }

    // Insert elements from an initializer list at the specified index
    iterator insert(const_iterator index, std::initializer_list<data_type> ilist) {
        return iterator;
    }

    // Insert elements from a range of iterators at the specified index
    template <class InputIt>
    iterator insert(const_iterator index, InputIt first, InputIt last) {
        return iterator;
    }

    template <class... Args>
    iterator emplace(const_iterator index, Args&&... args) {
        return iterator;
    }

    template <class... Args>
    data_type& emplace_front(Args&&... args) {
        return data_type;
    }

    template <class... Args>
    data_type& emplace_back(Args&&... args) {
        return data_type;
    }

    int remove(const data_type& value) {
        return 0;
    }

    template <class UnaryPredicate>
    int remove_if(UnaryPredicate p) {
        return 0;
    }

    iterator erase(const_iterator index) {
        return iterator;
    }

    iterator erase(const_iterator first, const_iterator last) {
        return iterator;
    }

    // Removes all elements from the linked list
    void clear() noexcept {}

    // Removes first element in the linked list
    void pop_front() {}

    // Removes last element in the linked list
    void pop_back() {}

    // Sorts elements in ascending order
    void sort() {}

    template <class Compare>
    void sort(Compare comp) {}

    // Removes duplicate elements from the linked list
    size_type unique() {
        return size_type;
    }

    // Removes duplicate elements from the linked list that satisfy predicate
    template <class BinaryPredicate>
    size_type unique(BinaryPredicate p) {
        return size_type;
    }

    void resize(size_type size) {}

    void resize(size_type size, const data_type& value) {}

    // Swaps head, tail, size and alloc with other
    void swap(DoubleLinkedList& other) noexcept {}

    // Reverses the order of elements in the linked list by swapping head and tail pointers
    void reverse() noexcept {}

    // Move all the elements of other to this list before the element at the given index
    void splice(const_iterator index, DoubleLinkedList& other) {}

    // Move all the elements of other to this list before the element at the given index
    void splice(const_iterator index, DoubleLinkedList&& other) {}

    // Move the element at it in other to this list before the element at the given index
    void splice(const_iterator index, DoubleLinkedList& other, const_iterator it) {}

    // Move the element at it in other to this list before the element at the given index
    void splice(const_iterator index, DoubleLinkedList&& other, const_iterator it) {}

    void splice(const_iterator index, DoubleLinkedList& other, const_iterator first, const_iterator last) {}

    void splice(const_iterator index, DoubleLinkedList&& other, const_iterator first, const_iterator last) {}

    void merge(DoubleLinkedList& other) {}

    void merge(DoubleLinkedList&& other) {}

    template <class Compare>
    void merge(DoubleLinkedList& other, Compare comp) {}

    template<class Compare>
    void merge(DoubleLinkedList&& other, Compare comp) {}

private:
    // Returns a pointer to Node in the DoubleLinkedList which is specified by the iterator it
    Node* getNodePtr(const_iterator it) const {
        auto index = std::distance(cbegin(), it);
        auto currentNode = head.get();

        for (std::size_t i = 0; i < index; ++i) {
            currentNode = currentNode->next.get();
        }

        return currentNode;
    }

    void deleteNode(node_type node) {
        std::allocator_traits<allocator_type>::destroy(m_allocator, node.get());
        std::allocator_traits<allocator_type>::deallocate(m_allocator, node.get(), 1);
    }

    node_type head;
    node_type tail;
    size_type m_size;
    allocator_type m_allocator;
};