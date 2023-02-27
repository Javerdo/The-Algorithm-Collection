#include <doctest/doctest.h>
#include <algorithmCollection/data structures/dynamicArray.h>

TEST_CASE("Test default constructor") {
    DynamicArray<int> arr;
    CHECK(arr.empty());
    CHECK(arr.size() == 0);
}

TEST_CASE("Test size constructor") {
    DynamicArray<int> arr(10);
    CHECK(!arr.empty());
    CHECK(arr.size() == 10);
}

TEST_CASE("Test initializer list constructor") {
    DynamicArray<int> arr = {1, 2, 3, 4, 5};
    CHECK(arr.size() == 5);
    CHECK(arr[0] == 1);
    CHECK(arr[4] == 5);
}

TEST_CASE("Test span constructor") {
    int data[] = {1, 2, 3, 4, 5};
    DynamicArray<int> arr(std::span(data, 5));
    CHECK(arr.size() == 5);
    CHECK(arr[0] == 1);
    CHECK(arr[4] == 5);
}

TEST_CASE("Test copy constructor") {
    DynamicArray<int> arr1 = {1, 2, 3, 4, 5};
    DynamicArray<int> arr2(arr1);
    CHECK(arr1.size() == arr2.size());
    CHECK(arr1[0] == arr2[0]);
    CHECK(arr1[4] == arr2[4]);
}

TEST_CASE("Test move constructor") {
    DynamicArray<int> arr1 = {1, 2, 3, 4, 5};
    DynamicArray<int> arr2(std::move(arr1));
    CHECK(arr1.empty());
    CHECK(arr2.size() == 5);
    CHECK(arr2[0] == 1);
    CHECK(arr2[4] == 5);
}

TEST_CASE("Test push_back function") {
    DynamicArray<int> arr;
    arr.push_back(1);
    CHECK(arr.size() == 1);
    CHECK(arr[0] == 1);
    arr.push_back(2);
    CHECK(arr.size() == 2);
    CHECK(arr[1] == 2);
}

TEST_CASE("Test push_front function") {
    DynamicArray<int> arr = {1, 2, 3, 4, 5};
    arr.push_front(0);
    CHECK(arr.size() == 6);
    CHECK(arr[0] == 0);
}

TEST_CASE("Test pop_back function") {
    DynamicArray<int> arr = {1, 2, 3, 4, 5};
    arr.pop_back();
    CHECK(arr.size() == 4);
    CHECK(arr[3] == 4);
    arr.pop_back();
    CHECK(arr.size() == 3);
    CHECK(arr[2] == 3);
}

TEST_CASE("Test insert function") {
    DynamicArray<int> arr = {1, 2, 3, 4, 5};
    auto it = arr.begin() + 2;
    arr.insert(it, 6);
    CHECK(arr.size() == 6);
}

TEST_CASE("Test data function") {
    DynamicArray<int> arr = {1, 2, 3, 4, 5};
    CHECK(arr.data().front() == 1);
    CHECK(arr.data().back() == 5);
}

TEST_CASE("Test at function") {
    DynamicArray<int> arr = {1, 2, 3, 4, 5};

    SUBCASE("Valid index") {
        CHECK_NOTHROW(arr.at(0));
        CHECK_NOTHROW(arr.at(arr.size() - 1));
    }

    SUBCASE("Invalid index") {
        CHECK_THROWS_AS(arr.at(arr.size()), std::out_of_range);
        CHECK_THROWS_WITH(arr.at(arr.size()), "Index out of range");
    }
}

TEST_CASE("Test reserve function") {
    DynamicArray<int> arr;
    arr.reserve(10);

    SUBCASE("Capacity increase") {
        CHECK(arr.capacity() == 10);
    }

    SUBCASE("Capacity unchanged if new_capacity is less than or equal to size") {
        arr.reserve(2);
        CHECK(arr.capacity() == 10);
    }
}

TEST_CASE("Test erase 1 element function") {
    DynamicArray<int> arr;

    arr.push_back(1);
    arr.push_back(2);
    arr.push_back(3);
    arr.push_back(4);

    // Test removing an element at the middle
    auto it1 = arr.begin() + 1;
    arr.erase(it1);
    DynamicArray<int> expected = {1, 3, 4};
    CHECK(arr == expected);

    // Test removing an element at the end
    auto it2 = arr.begin() + 2;
    arr.erase(it2);
    expected = {1, 3};
    CHECK(arr == expected);

    // Test removing an element at the beginning
    auto it3 = arr.begin();
    arr.erase(it3);
    expected = {3};
    CHECK(arr == expected);

    // Test removing the only element in the array
    auto it4 = arr.begin();
    arr.erase(it4);
    expected = {};
    CHECK(arr == expected);

    // Test removing from an empty array
    auto it5 = arr.begin();
    CHECK_THROWS(arr.erase(it5));
}