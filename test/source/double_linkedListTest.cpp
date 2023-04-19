#include <doctest/doctest.h>
#include <algorithmCollection/data structures/dynamicArray.h>
#include <algorithmCollection/data structures/double_linkedList.h>
#include <iostream>

TEST_CASE("Testing push_back with lvalue") {
    DoubleLinkedList<int> list;

    list.push_back(1);
    CHECK(list.size() == 1);
    CHECK(list.front().data == 1);
    CHECK(list.back().data == 1);

    list.push_back(2);
    CHECK(list.size() == 2);
    CHECK(list.front().data == 1);
    CHECK(list.back().data == 2);

    list.push_back(3);
    CHECK(list.size() == 3);
    CHECK(list.front().data == 1);
    CHECK(list.back().data == 3);
}

TEST_CASE("Testing push_back with rvalue") {
    DoubleLinkedList<int> list;

    list.push_back(1);
    CHECK(list.size() == 1);
    CHECK(list.front().data == 1);
    CHECK(list.back().data == 1);

    int val2 = 2;
    list.push_back(std::move(val2));
    CHECK(list.size() == 2);
    CHECK(list.front().data == 1);
    CHECK(list.back().data == 2);

    int val3 = 3;
    list.push_back(std::move(val3));
    CHECK(list.size() == 3);
    CHECK(list.front().data == 1);
    CHECK(list.back().data == 3);
}

TEST_CASE("Testing push_front with lvalue") {
    DoubleLinkedList<int> list;

    // Test push_front with lvalue
    int lvalue = 10;
    list.push_front(lvalue);
    REQUIRE(list.size() == 1);
    REQUIRE(list.front().data == 10);
    REQUIRE(list.back().data == 10);

    // Test push_front with lvalue again
    int another_lvalue = 30;
    list.push_front(another_lvalue);
    REQUIRE(list.size() == 2);
    REQUIRE(list.front().data == 30);
    REQUIRE(list.back().data == 10);
}

TEST_CASE("Testing push_front with rvalue") {
    DoubleLinkedList<int> list;

    // Test push_front with rvalue
    list.push_front(20);
    REQUIRE(list.size() == 1);
    REQUIRE(list.front().data == 20);
    REQUIRE(list.back().data == 20);

    // Test push_front with rvalue again
    list.push_front(40);
    REQUIRE(list.size() == 2);
    REQUIRE(list.front().data == 40);
    REQUIRE(list.back().data == 20);
}

TEST_CASE("Test inserting a single element at the specified index (lvalue)") {
    DoubleLinkedList<int> list;

    // Insert into an empty list
    int value = 42;
    list.insert(list.cbegin(), value);
    CHECK(list.front().data == 42);
    CHECK(--list.back().data == 42);
    CHECK(list.size() == 1);

    // Insert at the beginning
    value = 24;
    list.insert(list.cbegin(), value);
    CHECK(list.front().data == 24);
    CHECK(--list.back().data == 42);
    CHECK(list.size() == 2);

    // Insert at the end
    value = 99;
    list.insert(list.cend(), value);
    CHECK(list.front().data == 24);
    CHECK(--list.back().data == 99);
    CHECK(list.size() == 3);

    // Insert in the middle
    auto it = list.cbegin();
    ++it;
    value = 55;
    list.insert(it, value);
    CHECK(list.front().data == 24);
    CHECK(--list.back().data == 99);
    CHECK(list.size() == 4);
    it = list.cbegin();
    ++it;
    CHECK(*it == 55);
}

/*
TEST_CASE("Test inserting a single element at the specified index (rvalue)") {
    DoubleLinkedList<int> list;

    // Insert into an empty list
    list.insert(list.cbegin(), 42);
    CHECK(list.front().data == 42);
    CHECK(--list.back().data == 42);
    CHECK(list.size() == 1);

    // Insert at the beginning
    list.insert(list.cbegin(), 24);
    CHECK(list.front().data == 24);
    CHECK(--list.back().data == 42);
    CHECK(list.size() == 2);

    // Insert at the end
    list.insert(list.cend(), 99);
    CHECK(list.front().data == 24);
    CHECK(--list.back().data == 99);
    CHECK(list.size() == 3);

    // Insert in the middle
    auto it = list.cbegin();
    ++it;
    list.insert(it, 55);
    CHECK(list.front().data == 24);
    CHECK(--list.back().data == 99);
    CHECK(list.size() == 4);
    it = list.cbegin();
    ++it;
    CHECK(*it == 55);
}
*/

/*
TEST_CASE("Test inserting multiple copies of an element at the specified index") {
    DoubleLinkedList<int> list;

    // Insert multiple elements at the beginning
    list.insert(list.begin(), 3, 42);
    CHECK(list.front().data == 42);
    CHECK(--list.back().data == 42);
    CHECK(list.size() == 3);

    // Check if all elements are the same
    for (const auto& elem : list) {
        CHECK(elem == 42);
    }
}
*/

/*
TEST_CASE("Test inserting elements from an initializer list at the specified index") {
    DoubleLinkedList<int> list;

    // Insert initializer list at the beginning
    list.insert(list.begin(), {1, 2, 3});
    CHECK(list.front().data == 1);
    CHECK(list.back().data == 3);
    CHECK(list.size() == 3);

    // Insert initializer list in the middle
    auto it = list.begin();
    ++it;
    list.insert(it, {4, 5});
    CHECK(list.front().data == 1);
    CHECK(list.back().data == 3);
    CHECK(list.size() == 5);

    DynamicArray<int> expected = {1, 4, 5, 2, 3};
    int idx = 0;
    for (const auto& elem : list) {
        CHECK(elem == expected[idx]);
        ++idx;
    }

    // Insert initializer list at the end
    list.insert(list.end(), {6, 7});
    CHECK(list.front().data == 1);
    CHECK(list.back().data == 7);
    CHECK(list.size() == 7);

    expected = {1, 4, 5, 2, 3, 6, 7};
    idx = 0;
    for (const auto& elem : list) {
        CHECK(elem == expected[idx]);
        ++idx;
    }
}
*/

/*
TEST_CASE("Test inserting elements from a range of iterators at the specified index") {
    DoubleLinkedList<int> list;
    DynamicArray<int> values = {1, 2, 3};

    // Insert elements from a range at the beginning
    list.insert(list.begin(), values.begin(), values.end());
    CHECK(list.front().data == 1);
    CHECK(list.back().data == 3);
    CHECK(list.size() == 3);

    // Insert elements from a range in the middle
    DynamicArray<int> new_values = {4, 5};
    auto it = list.begin();
    ++it;
    list.insert(it, new_values.begin(), new_values.end());
    CHECK(list.front().data == 1);
    CHECK(list.back().data == 3);
    CHECK(list.size() == 5);

    DynamicArray<int> expected = {1, 4, 5, 2, 3};
    int idx = 0;
    for (const auto& elem : list) {
        CHECK(elem == expected[idx]);
        ++idx;
    }

    // Insert elements from a range at the end
    DynamicArray<int> end_values = {6, 7};
    list.insert(list.end(), end_values.begin(), end_values.end());
    CHECK(list.front().data == 1);
    CHECK(list.back().data == 7);
    CHECK(list.size() == 7);

    expected = {1, 4, 5, 2, 3, 6, 7};
    idx = 0;
    for (const auto& elem : list) {
        CHECK(elem == expected[idx]);
        ++idx;
    }
}
*/