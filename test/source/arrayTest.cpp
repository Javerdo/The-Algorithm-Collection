#include <doctest/doctest.h>
#include <algorithm/array.h>

TEST_CASE("Test default constructor") {
    Array<int> arr;
    CHECK(arr.empty());
    CHECK(arr.size() == 0);
}

TEST_CASE("Test size constructor") {
    Array<int> arr(10);
    CHECK(!arr.empty());
    CHECK(arr.size() == 10);
}

TEST_CASE("Test initializer list constructor") {
    Array<int> arr = {1, 2, 3, 4, 5};
    CHECK(arr.size() == 5);
    CHECK(arr[0] == 1);
    CHECK(arr[4] == 5);
}

TEST_CASE("Test span constructor") {
    int data[] = {1, 2, 3, 4, 5};
    Array<int> arr(std::span(data, 5));
    CHECK(arr.size() == 5);
    CHECK(arr[0] == 1);
    CHECK(arr[4] == 5);
}

TEST_CASE("Test copy constructor") {
    Array<int> arr1 = {1, 2, 3, 4, 5};
    Array<int> arr2(arr1);
    CHECK(arr1.size() == arr2.size());
    CHECK(arr1[0] == arr2[0]);
    CHECK(arr1[4] == arr2[4]);
}

TEST_CASE("Test move constructor") {
    Array<int> arr1 = {1, 2, 3, 4, 5};
    Array<int> arr2(std::move(arr1));
    CHECK(arr1.empty());
    CHECK(arr2.size() == 5);
    CHECK(arr2[0] == 1);
    CHECK(arr2[4] == 5);
}

TEST_CASE("Test push_back") {
    Array<int> arr;
    arr.push_back(1);
    CHECK(arr.size() == 1);
    CHECK(arr[0] == 1);
    arr.push_back(2);
    CHECK(arr.size() == 2);
    CHECK(arr[1] == 2);
}

TEST_CASE("Test pop_back") {
    Array<int> arr = {1, 2, 3, 4, 5};
    arr.pop_back();
    CHECK(arr.size() == 4);
    CHECK(arr[3] == 4);
    arr.pop_back();
    CHECK(arr.size() == 3);
    CHECK(arr[2] == 3);
}

TEST_CASE("Test insert") {
    Array<int> arr = {1, 2, 3, 4, 5};
    arr.insert(2, 6);
    CHECK(arr.size() == 6);
}