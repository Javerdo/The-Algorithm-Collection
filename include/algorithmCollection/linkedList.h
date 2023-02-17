#pragma once
#include <memory>
#include "simpleAllocator.h"

template <typename T, typename Alloc = SimpleAllocator<T>>
class LinkedList {
    struct Node {
        T data;
        std::unique_ptr<Node, std::function<void(Node*)>> next;
        std::unique_ptr<Node, std::function<void(Node*)>> prev;
    };

public:
    LinkedList() : head(nullptr), tail(nullptr), count(0), allocator() {}
    ~LinkedList() = default;

    std::size_t size() const { return count; }
    bool empty() const { return count == 0; }

    // Adds an element to the end of the linked list
    void push_back(const T& value) {
        auto newNode = std::unique_ptr<Node, std::function<void(Node*)>>(allocator.allocate(1),
                                    [&](Node* n) { allocator.deallocate(n, 1); });
        allocator.construct(newNode.get(), Node{value, nullptr});
    }

    // Adds an element to the front of the linked list
    void push_front(const T& value) {
        auto newNode = std::unique_ptr<Node, std::function<void(Node*)>>(allocator.allocate(1),
                                    [&](Node* n) { allocator.deallocate(n, 1); });
        allocator.construct(newNode.get(), Node{value, std::move(head)});
    }

    // Removes all elements from the linked list
    void clear() {}

    void pop_front() {}

private:
    std::unique_ptr<Node, std::function<void(Node*)>> head;
    std::unique_ptr<Node, std::function<void(Node*)>> tail;
    std::size_t count;
    Alloc allocator;
};