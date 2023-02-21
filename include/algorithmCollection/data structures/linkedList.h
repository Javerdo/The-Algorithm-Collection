#pragma once
#include <memory>
#include <stdexcept>
#include <compare>
#include <algorithm>
#include <functional>
#include "../allocators/simpleAllocator.h"

// Double linked list with custom memory allocation and safety
template <typename T, typename Alloc = SimpleAllocator<T>>
class DoubleLinkedList {
private:
    struct Node {
        T data;
        std::unique_ptr<Node, std::function<void(Node*)>> next;
        std::unique_ptr<Node, std::function<void(Node*)>> prev;

        Node(T value, std::unique_ptr<Node> n = nullptr, std::unique_ptr<Node> p = nullptr)
            : data(value), next(std::move(n), [](Node* n) {}), prev(std::move(p), [](Node* n) {}) {}
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

    explicit DoubleLinkedList(const Alloc& alloc = Alloc())
        : allocator(alloc), head(nullptr), tail(nullptr), m_size(0) {}

    DoubleLinkedList(std::size_t size, const T& value, const Alloc& alloc = alloc())
        : allocator(alloc), head(nullptr), tail(nullptr), m_size(0) {
        for (std::size_t i = 0; i < size; ++i) {
            push_back(value);
        }
    }

    explicit DoubleLinkedList(std::size_t size, const Alloc& alloc = alloc())
        : allocator(alloc), head(nullptr), tail(nullptr), m_size(0) {}

    template <class InputIt>
    DoubleLinkedList(InputIt first, InputIt last, const Alloc& alloc = alloc())
        : allocator(alloc), head(nullptr), tail(nullptr), m_size(0) {
        while (first != last) {
            push_back(*first);
            ++first;
        }
    }

    DoubleLinkedList(const DoubleLinkedList& other)
        : DoubleLinkedList(other, other.get_allocator()) {}

    DoubleLinkedList(const DoubleLinkedList& other, const Alloc& alloc)
        : allocator(alloc), head(nullptr), tail(nullptr), m_size(0) {
        for (const auto& value : other) {
            push_back(value);
        }
    }

    DoubleLinkedList(DoubleLinkedList&& other)
        : allocator(std::move(other.get_allocator())), head(std::move(other.head)),
        tail(std::move(other.tail)), m_size(other.m_size) {
        other.head = nullptr;
        other.tail = nullptr;
        other.m_size = 0;
    }

    DoubleLinkedList(DoubleLinkedList&& other, const Alloc& alloc)
        : allocator(alloc), head(nullptr), tail(nullptr), m_size(0) {
        if (allocator == other.get_allocator()) {
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

    DoubleLinkedList(std::initializer_list<T> init, const Alloc& alloc = Alloc())
        : DoubleLinkedList(init.begin(), init.end(), alloc) {}

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
            allocator = std::move(other.allocator);
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
    DoubleLinkedList& operator=(std::initializer_list<T> ilist) {
        clear();
        for (const auto& value : ilist) {
            push_back(value);
        }

        return *this;
    }

    // Comparison operator
    template <class T1, class Alloc>
    friend std::strong_ordering operator<=>(
        const DoubleLinkedList<T1, Alloc>& lhs, const DoubleLinkedList<T1, Alloc>& rhs) {
        auto l_iter = lhs.begin(), l_end = lhs.end();
        auto r_iter = rhs.begin(), r_end = rhs.end();

        while (l_iter != l_end && r_iter != r_end) {
            if (*l_iter < *r_iter) {
                return std::strong_ordering::less;
            }
            else if (*r_iter < *l_iter) {
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

    std::size_t size() const noexcept { return m_size; }
    [[nodiscard]] constexpr bool empty() const noexcept { return m_size == 0; }

    // Returns allocator associated with the linked list
    Alloc get_allocator() const noexcept { return allocator; }

    // Get first element in linked list
    std::unique_ptr<Node, std::function<void(Node*)>> front() { return head; }
    constexpr std::unique_ptr<Node, std::function<void(Node*)>> front() const { return head; }

    // Get last element in linked list
    std::unique_ptr<Node, std::function<void(Node*)>> back() { return tail; }
    constexpr std::unique_ptr<Node, std::function<void(Node*)>> back() const { return tail; }

    // Replace elements with copies of value
    void assign(std::size_t size, const T& value) {
        clear();
        for (std::size_t i = 0; i < size; ++i) {
            push_back(value);
        }
    }

    // Replace elements with copies in the range (first, last)
    template <class InputIt>
    void assign(InputIt first, InputIt last) {
        clear();
        for (auto it = first; it != last; ++it) {
            push_back(*it);
        }
    }

    // Replace elements with copies from initializer list
    void assign(std::initializer_list<T> ilist) {
        clear();
        for (const auto& value : ilist) {
            push_back(value);
        }
    }

    // Adds an element to the end of the linked list
    void push_back(const T& value) {
        using allocator_type = typename std::allocator_traits<Alloc>::template rebind_alloc<Node>;
        allocator_type node_allocator(allocator);

        std::unique_ptr<Node, std::function<void(Node*)>> newNode(
            std::allocator_traits<allocator_type>::allocate(node_allocator, 1),
            [&](Node* n) {
                std::allocator_traits<allocator_type>::destroy(node_allocator, n);
        std::allocator_traits<allocator_type>::deallocate(node_allocator, n, 1);
            }
        );

        std::allocator_traits<allocator_type>::construct(node_allocator, newNode.get(), Node{ value, nullptr, std::move(tail) });
        auto newTail = newNode.release();

        if (tail) {
            tail->next = std::unique_ptr<Node, std::function<void(Node*)>>(newTail, [](Node*) {});
        }
        else {
            head = std::unique_ptr<Node, std::function<void(Node*)>>(newTail, [](Node*) {});
        }
        tail = std::unique_ptr<Node, std::function<void(Node*)>>(newTail, [](Node*) {});
        ++m_size;
    }

    void push_back(T&& value) {
        using allocator_type = typename std::allocator_traits<Alloc>::template rebind_alloc<Node>;
        allocator_type node_allocator(allocator);

        std::unique_ptr<Node, std::function<void(Node*)>> newNode(
            std::allocator_traits<allocator_type>::allocate(node_allocator, 1),
            [&](Node* n) {
                std::allocator_traits<allocator_type>::destroy(node_allocator, n);
        std::allocator_traits<allocator_type>::deallocate(node_allocator, n, 1);
            }
        );

        std::allocator_traits<allocator_type>::construct(node_allocator, newNode.get(), Node{ std::move(value), nullptr, std::move(tail) });
        auto newTail = newNode.release();

        if (tail) {
            tail->next = std::unique_ptr<Node, std::function<void(Node*)>>(newTail, [](Node*) {});
        }
        else {
            head = std::unique_ptr<Node, std::function<void(Node*)>>(newTail, [](Node*) {});
        }
        tail = std::unique_ptr<Node, std::function<void(Node*)>>(newTail, [](Node*) {});
        ++m_size;
    }

    // Adds an element to the front of the linked list
    void push_front(const T& value) {
        using allocator_type = typename std::allocator_traits<Alloc>::template rebind_alloc<Node>;
        allocator_type node_allocator(allocator);

        std::unique_ptr<Node, std::function<void(Node*)>> newNode(
            std::allocator_traits<allocator_type>::allocate(node_allocator, 1),
            [&](Node* n) {
                std::allocator_traits<allocator_type>::destroy(node_allocator, n);
        std::allocator_traits<allocator_type>::deallocate(node_allocator, n, 1);
            }
        );

        std::allocator_traits<allocator_type>::construct(node_allocator, newNode.get(), Node{ value, std::move(head), nullptr });
        auto newHead = newNode.release();

        if (head) {
            head.get()->prev = std::unique_ptr<Node, std::function<void(Node*)>>(newHead, [](Node*) {});
        }
        else {
            tail = std::unique_ptr<Node, std::function<void(Node*)>>(newHead, [](Node*) {});
        }
        head = std::unique_ptr<Node, std::function<void(Node*)>>(newHead, [](Node*) {});
        ++m_size;
    }

    void push_front(T&& value) {
        using allocator_type = typename std::allocator_traits<Alloc>::template rebind_alloc<Node>;
        allocator_type node_allocator(allocator);

        std::unique_ptr<Node, std::function<void(Node*)>> newNode(
            std::allocator_traits<allocator_type>::allocate(node_allocator, 1),
            [&](Node* n) {
                std::allocator_traits<allocator_type>::destroy(node_allocator, n);
                std::allocator_traits<allocator_type>::deallocate(node_allocator, n, 1);
            }
        );

        std::allocator_traits<allocator_type>::construct(node_allocator, newNode.get(), Node{ std::move(value), std::move(head), nullptr });
        auto newHead = newNode.release();

        if (head) {
            head.get()->prev = std::unique_ptr<Node, std::function<void(Node*)>>(newHead, [](Node*) {});
        }
        else {
            tail = std::unique_ptr<Node, std::function<void(Node*)>>(newHead, [](Node*) {});
        }
        head = std::unique_ptr<Node, std::function<void(Node*)>>(newHead, [](Node*) {});
        ++m_size;
    }

    // Insert a single element at the specified index
    iterator insert(const_iterator index, const T& value) {
        if (index == end()) {
            push_back(value);
            return tail.get();
        }
        else if (index == begin()) {
            push_front(value);
            return head.get();
        }
        else {
            using allocator_type = typename std::allocator_traits<Alloc>::template rebind_alloc<Node>;
            allocator_type node_allocator(allocator);

            std::unique_ptr<Node, std::function<void(Node*)>> newNode(
                std::allocator_traits<allocator_type>::allocate(node_allocator, 1),
                [&](Node* n) {
                    std::allocator_traits<allocator_type>::destroy(node_allocator, n);
                    std::allocator_traits<allocator_type>::deallocate(node_allocator, n, 1);
                }
            );

            auto nextNode = index;
            auto prevNode = nextNode->prev.get();

            std::allocator_traits<allocator_type>::construct(node_allocator, newNode.get(),
                Node{ value, std::move(nextNode),
                std::unique_ptr<Node, std::function<void(Node*)>>(prevNode, [](Node*) {}) });
            nextNode->prev = std::unique_ptr<Node, std::function<void(Node*)>>(newNode.get(), [](Node*) {});
            prevNode->next = std::unique_ptr<Node, std::function<void(Node*)>>(newNode.get(), [](Node*) {});
            auto result = newNode.release();

            ++m_size;

            return iterator(result);
        }
    }

    iterator insert(const_iterator index, T&& value) {
        if (index == end()) {
            push_back(std::move(value));
            return tail.get();
        }
        else if (index == begin()) {
            push_front(std::move(value));
            return head.get();
        }
        else {
            using allocator_type = typename std::allocator_traits<Alloc>::template rebind_alloc<Node>;
            allocator_type node_allocator(allocator);

            std::unique_ptr<Node, std::function<void(Node*)>> newNode(
                std::allocator_traits<allocator_type>::allocate(node_allocator, 1),
                [&](Node* n) {
                    std::allocator_traits<allocator_type>::destroy(node_allocator, n);
                    std::allocator_traits<allocator_type>::deallocate(node_allocator, n, 1);
                }
            );

            auto nextNode = index;
            auto prevNode = nextNode->prev.get();

            std::allocator_traits<allocator_type>::construct(node_allocator, newNode.get(),
                Node{ std::move(value),
                std::move(nextNode),
                std::unique_ptr<Node,
                std::function<void(Node*)>>(prevNode, [](Node*) {}) });
            nextNode->prev = std::unique_ptr<Node, std::function<void(Node*)>>(newNode.get(), [](Node*) {});
            prevNode->next = std::unique_ptr<Node, std::function<void(Node*)>>(newNode.get(), [](Node*) {});
            auto result = newNode.release();

            ++m_size;

            return iterator(result);
        }
    }

    // Insert multiple copies of an element at the specified index
    iterator insert(const_iterator index, std::size_t size, const T& value) {
        for (std::size_t i = 0; i < size; ++i) {
            index = insert(index, value);
            ++index;
        }

        return iterator(index);
    }

    // Insert elements from an initializer list at the specified index
    iterator insert(const_iterator index, std::initializer_list<T> ilist) {
        for (const auto& value : ilist) {
            index = insert(index, value);
            ++index;
        }

        return iterator(index);
    }

    // Insert elements from a range of iterators at the specified index
    template <class InputIt>
    iterator insert(const_iterator index, InputIt first, InputIt last) {
        while (first != last) {
            index = insert(index, *first);
            ++first;
            ++index;
        }

        return iterator(index);
    }

    template <class... Args>
    iterator emplace(const_iterator index, Args&&... args) {
        Node* currentNode = const_cast<Node*>(index);

        using allocator_type = typename std::allocator_traits<Alloc>::template rebind_alloc<Node>;
        allocator_type node_allocator(allocator);

        std::unique_ptr<Node, std::function<void(Node*)>> newNode(
            std::allocator_traits<allocator_type>::allocate(node_allocator, 1),
            [&](Node* n) {
                std::allocator_traits<allocator_type>::destroy(node_allocator, n);
                std::allocator_traits<allocator_type>::deallocate(node_allocator, n, 1);
            }
        );

        std::allocator_traits<allocator_type>::construct(node_allocator, newNode.get(),
            Node{ std::forward<Args>(args)...,
            std::move(currentNode->prev),
            std::unique_ptr<Node,
            std::function<void(Node*)>>(currentNode, [](Node*) {}) });

        Node* newElement = newNode.get();
        if (newElement->prev) {
            newElement->prev->next = std::move(newNode);
        }
        else {
            head = std::move(newNode);
        }

        currentNode->prev = std::unique_ptr<Node, std::function<void(Node*)>>(newElement, [](Node*) {});
        ++m_size;

        return iterator(newElement);
    }

    template <class... Args>
    T& emplace_front(Args&&... args) {
        using allocator_type = typename std::allocator_traits<Alloc>::template rebind_alloc<Node>;
        allocator_type node_allocator(allocator);

        std::unique_ptr<Node, std::function<void(Node*)>> newNode(
            std::allocator_traits<allocator_type>::allocate(node_allocator, 1),
            [&](Node* n) {
                std::allocator_traits<allocator_type>::destroy(node_allocator, n);
                std::allocator_traits<allocator_type>::deallocate(node_allocator, n, 1);
            }
        );
        std::allocator_traits<allocator_type>::construct(node_allocator, newNode.get(),
            Node{ std::forward<Args>(args)...,
            std::move(head),
            nullptr });

        Node* newElement = newNode.get();
        if (head) {
            head.get()->prev = std::move(newNode);
        }
        else {
            tail = std::move(newNode);
        }
        head = std::move(newNode);
        ++m_size;

        return newElement->data;
    }

    template <class... Args>
    T& emplace_back(Args&&... args) {
        using allocator_type = typename std::allocator_traits<Alloc>::template rebind_alloc<Node>;
        allocator_type node_allocator(allocator);

        std::unique_ptr<Node, std::function<void(Node*)>> newNode(
            std::allocator_traits<allocator_type>::allocate(node_allocator, 1),
            [&](Node* n) {
                std::allocator_traits<allocator_type>::destroy(node_allocator, n);
                std::allocator_traits<allocator_type>::deallocate(node_allocator, n, 1);
            }
        );
        std::allocator_traits<allocator_type>::construct(node_allocator, newNode.get(),
            Node{ std::forward<Args>(args)...,
            nullptr,
            std::move(tail) });

        Node* newElement = newNode.get();
        if (tail) {
            tail->next = std::move(newNode);
        }
        else {
            head = std::move(newNode);
        }
        tail = std::move(newNode);
        ++m_size;

        return newElement->data;
    }

    std::size_t remove(const T& value) {
        std::size_t count = 0;
        iterator it = begin();

        while (it != end()) {
            if (*it == value) {
                it = erase(it);
                ++count;
            }
            else {
                ++it;
            }
        }
        return count;
    }

    template <class UnaryPredicate>
    std::size_t remove_if(UnaryPredicate p) {
        std::size_t count = 0;
        iterator it = begin();

        while (it != end()) {
            if (p(*it)) {
                it = erase(it);
                ++count;
            }
            else {
                ++it;
            }
        }
        return count;
    }

    iterator erase(const_iterator index) {
        if (index == cend() - 1) {
            return end();
        }

        iterator it = const_cast<iterator>(index);
        if (it == begin()) {
            pop_front();
            return begin();
        }
        else if (it == --end()) {
            pop_back();
            return end();
        }
        else {
            iterator prev = it->prev.get();
            iterator next = it->next.get();
            prev->next = std::move(it->next);
            next->prev = std::move(it->prev);
            --m_size;
            return next;
        }
    }

    iterator erase(const_iterator first, const_iterator last) {
        iterator it = const_cast<iterator>(first);

        while (it != last) {
            it = erase(it);
        }
        return const_cast<iterator>(last);
    }

    // Removes all elements from the linked list
    void clear() noexcept {
        if (empty()) { return; }
        while (head) {
            head = std::move(head.get()->next);
        }
        tail.reset();
        m_size = 0;
    }

    // Removes first element in the linked list
    void pop_front() {
        if (empty()) { return; }
        if (m_size == 1) {
            head.reset();
            tail.reset();
        }
        else {
            head = std::move(head.get()->next);
            head.get()->prev.reset();
        }
        --m_size;
    }

    // Removes last element in the linked list
    void pop_back() {
        if (empty()) { return; }
        if (m_size == 1) {
            head.reset();
            tail.reset();
        }
        else {
            tail = std::move(tail.get()->prev);
            tail.get()->next.reset();
        }
        --m_size;
    }

    // Sorts elements in ascending order
    void sort() {
        sort(std::less<T>());
    }

    template <class Compare>
    void sort(Compare comp) {
        if (size() <= 1) { return; }

        DoubleLinkedList left(allocator);
        DoubleLinkedList right(allocator);

        auto mid = begin();
        for (std::size_t i = 0; i < size() / 2; ++i) {
            ++mid;
        }

        std::for_each(begin(), mid, [&left](const T& value) {
            left.push_back(value);
            });

        std::for_each(mid, end(), [&right](const T& value) {
            right.push_back(value);
            });

        left.sort(comp);
        right.sort(comp);

        clear();

        while (!left.empty() && !right.empty()) {
            if (comp(left.front()->data, right.front()->data)) {
                push_back(std::move(left.front()->data));
                left.pop_front();
            }
            else {
                push_back(std::move(right.front()->data));
                right.pop_front();
            }
        }

        while (!left.empty()) {
            push_back(std::move(left.front()->data));
            left.pop_front();
        }

        while (!right.empty()) {
            push_back(std::move(right.front()->data));
            right.pop_front();
        }
    }

    // Removes duplicate elements from the linked list
    std::size_t unique() {
        if (m_size < 2) {
            return 0;
        }

        std::size_t num_removed = 0;
        auto prev = head.get();
        auto curr = prev->next.get();

        while (curr != nullptr) {
            if (curr->data == prev->data) {
                prev->next = std::move(curr->next);
                if (curr->next != nullptr) {
                    curr->next->prev = std::unique_ptr<Node, std::function<void(Node*)>>(prev, [](Node*) {});
                }
                --m_size;
                ++num_removed;
                curr = prev->next.get();
            }
            else {
                prev = curr;
                curr = curr->next.get();
            }
        }

        tail = std::unique_ptr<Node, std::function<void(Node*)>>(prev, [](Node*) {});
        return num_removed;
    }

    // Removes duplicate elements from the linked list that satisfy predicate
    template <class BinaryPredicate>
    std::size_t unique(BinaryPredicate p) {
        if (m_size < 2) {
            return 0;
        }

        std::size_t num_removed = 0;
        auto prev = head.get();
        auto curr = prev->next.get();

        while (curr != nullptr) {
            if (p(curr->data, prev->data)) {
                prev->next = std::move(curr->next);
                if (curr->next != nullptr) {
                    curr->next->prev = std::unique_ptr<Node, std::function<void(Node*)>>(prev, [](Node*) {});
                }
                --m_size;
                ++num_removed;
                curr = prev->next.get();
            }
            else {
                prev = curr;
                curr = curr->next.get();
            }
        }

        tail = std::unique_ptr<Node, std::function<void(Node*)>>(prev, [](Node*) {});
        return num_removed;
    }

    void resize(std::size_t size) {
        if (size < m_size) {
            while (m_size > size) {
                pop_back();
            }
        }
        else if (size > m_size) {
            std::size_t numToAdd = size - m_size;
            while (numToAdd--) {
                push_back(T{});
            }
        }
    }

    void resize(std::size_t size, const T& value) {
        if (size < m_size) {
            while (m_size > size) {
                pop_back();
            }
        }
        else if (size > m_size) {
            std::size_t numToAdd = size - m_size;
            while (numToAdd--) {
                push_back(value);
            }
        }
    }

    // Swaps head, tail, size and alloc with other
    void swap(DoubleLinkedList& other) noexcept {
        std::swap(head, other.head);
        std::swap(tail, other.tail);
        std::swap(m_size, other.m_size);
        std::swap(allocator, other.allocator);
    }

    // Reverses the order of elements in the linked list by swapping head and tail pointers
    void reverse() noexcept {
        if (empty() || m_size == 1) { return; }

        std::unique_ptr<Node, std::function<void(Node*)>> temp = std::move(head);
        head = std::move(tail);
        tail = std::move(temp);

        Node* current = head.get();
        while (current != nullptr) {
            std::unique_ptr<Node, std::function<void(Node*)>> temp = std::move(current->next);
            current->next = std::move(current->prev);
            current->prev = std::move(temp);
            current = current->prev.get();
        }
    }

    // Move all the elements of other to this list before the element at the given index
    void splice(const_iterator index, DoubleLinkedList& other) {
        splice(index, std::move(other));
    }

    // Move all the elements of other to this list before the element at the given index
    void splice(const_iterator index, DoubleLinkedList&& other) {
        if (other.empty()) { return; }
        if (index == cbegin()) {
            other.tail->next = std::move(head);
            if (head) {
                head.get()->prev = std::move(other.tail);
            }
            else {
                tail = std::move(other.tail);
            }
            head = std::move(other.head);
        }
        else {
            auto it = getNodePtr(index);
            auto& beforeNode = it->prev;
            beforeNode->next = std::move(other.head);
            other.head.get()->prev = std::move(beforeNode);
            it->prev = std::move(other.tail);
            other.tail->next = std::move(it);
        }
        m_size += other.m_size;
        other.m_size = 0;
        other.head = nullptr;
        other.tail = nullptr;
    }

    // Move the element at it in other to this list before the element at the given index
    void splice(const_iterator index, DoubleLinkedList& other, const_iterator it) {
        splice(index, std::move(other), it);
    }

    // Move the element at it in other to this list before the element at the given index
    void splice(const_iterator index, DoubleLinkedList&& other, const_iterator it) {
        if (it == other.cend()) { return; }
        auto node = getNodePtr(it);
        auto& beforeNode = getNodePtr(index);

        if (node == beforeNode) {
            return;
        }
        if (node->next == beforeNode) {
            beforeNode->prev = std::move(node);
            node->prev->next = std::move(beforeNode);
            beforeNode->next = std::move(node->next);
            node->next->prev = std::move(beforeNode);
        }
        else {
            auto& beforeNode = getNodePtr(index);
            auto& prevNode = node->prev;
            auto& nextNode = node->next;
            beforeNode->prev->next = std::move(nextNode);
            nextNode->prev = std::move(beforeNode->prev);
            beforeNode->prev = std::move(prevNode);
            prevNode->next = std::move(beforeNode);
            beforeNode->next = std::move(node);
            node->prev = std::move(beforeNode);
        }

        ++m_size;
        --other.m_size;
    }

    void splice(const_iterator index, DoubleLinkedList& other, const_iterator first, const_iterator last) {
        splice(index, std::move(other), first, last);
    }

    void splice(const_iterator index, DoubleLinkedList&& other, const_iterator first, const_iterator last) {
        // Check if the iterators are valid and first < last
        if (first == last) {
            return;
        }

        Node* prevNode = getNodePtr(index);
        Node* firstNode = other.getNodePtr(first);
        Node* lastNode = other.getNodePtr(last);

        if (first == other.begin() && last == other.end()) {
            other.head.reset();
            other.tail.reset();
            other.m_size = 0;
        }
        else {
            if (first == other.begin()) {
                other.head = std::move(lastNode->next);
                other.head.get()->prev.reset();
            }
            else {
                auto beforeFirst = other.getNodePtr(std::prev(first));
                beforeFirst->next = std::move(lastNode->next);
                lastNode->next->prev = std::unique_ptr<Node, std::function<void(Node*)>>(beforeFirst, [](Node*) {});
            }
            other.m_size -= std::distance(first, last);
        }

        if (prevNode == head.get()) {
            head = std::unique_ptr<Node, std::function<void(Node*)>>(firstNode, [](Node*) {});
            firstNode->prev.reset();
        }
        else {
            auto beforePrev = prevNode->prev;
            beforePrev->next = std::unique_ptr<Node, std::function<void(Node*)>>(firstNode, [](Node*) {});
            firstNode->prev = std::move(beforePrev);
        }
        prevNode->prev = std::unique_ptr<Node, std::function<void(Node*)>>(lastNode, [](Node*) {});
        lastNode->next = std::unique_ptr<Node, std::function<void(Node*)>>(prevNode, [](Node*) {});
        m_size += std::distance(first, last);
    }

    void merge(DoubleLinkedList& other) {
        merge(other, std::less<T>());
    }

    void merge(DoubleLinkedList&& other) {
        merge(other, std::less<T>());
    }

    template <class Compare>
    void merge(DoubleLinkedList& other, Compare comp) {
        auto it1 = begin();
        auto it2 = other.begin();

        while (it1 != end() && it2 != other.end()) {
            if (comp(*it2, *it1)) {
                auto next_it2 = it2->next.get();
                splice(it1, other, it2);
                it2 = next_it2;
            }
            else {
                ++it1;
            }
        }

        if (it2 != other.end()) {
            splice(end(), other, it2, other.end());
        }
    }

    template<class Compare>
    void merge(DoubleLinkedList&& other, Compare comp) {
        merge(other, comp);
    }

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

    std::unique_ptr<Node, std::function<void(Node*)>> head;
    std::unique_ptr<Node, std::function<void(Node*)>> tail;
    std::size_t m_size;
    Alloc allocator;
};