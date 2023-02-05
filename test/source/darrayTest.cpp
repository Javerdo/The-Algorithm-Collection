#include <doctest/doctest.h>
#include <algorithmCollection/darray.h>

TEST_CASE("Test default constructor") {
    Darray<int> arr;
    CHECK(arr.empty());
    CHECK(arr.size() == 0);
}

TEST_CASE("Test size constructor") {
    Darray<int> arr(10);
    CHECK(!arr.empty());
    CHECK(arr.size() == 10);
}

TEST_CASE("Test initializer list constructor") {
    Darray<int> arr = {1, 2, 3, 4, 5};
    CHECK(arr.size() == 5);
    CHECK(arr[0] == 1);
    CHECK(arr[4] == 5);
}

TEST_CASE("Test span constructor") {
    int data[] = {1, 2, 3, 4, 5};
    Darray<int> arr(std::span(data, 5));
    CHECK(arr.size() == 5);
    CHECK(arr[0] == 1);
    CHECK(arr[4] == 5);
}

TEST_CASE("Test copy constructor") {
    Darray<int> arr1 = {1, 2, 3, 4, 5};
    Darray<int> arr2(arr1);
    CHECK(arr1.size() == arr2.size());
    CHECK(arr1[0] == arr2[0]);
    CHECK(arr1[4] == arr2[4]);
}

TEST_CASE("Test move constructor") {
    Darray<int> arr1 = {1, 2, 3, 4, 5};
    Darray<int> arr2(std::move(arr1));
    CHECK(arr1.empty());
    CHECK(arr2.size() == 5);
    CHECK(arr2[0] == 1);
    CHECK(arr2[4] == 5);
}

TEST_CASE("Test push_back function") {
    Darray<int> arr;
    arr.push_back(1);
    CHECK(arr.size() == 1);
    CHECK(arr[0] == 1);
    arr.push_back(2);
    CHECK(arr.size() == 2);
    CHECK(arr[1] == 2);
}

TEST_CASE("Test pop_back function") {
    Darray<int> arr = {1, 2, 3, 4, 5};
    arr.pop_back();
    CHECK(arr.size() == 4);
    CHECK(arr[3] == 4);
    arr.pop_back();
    CHECK(arr.size() == 3);
    CHECK(arr[2] == 3);
}

TEST_CASE("Test insert function") {
    Darray<int> arr = {1, 2, 3, 4, 5};
    arr.insert(2, 6);
    CHECK(arr.size() == 6);
}

TEST_CASE("Test data function") {
    Darray<int> arr = {1, 2, 3, 4, 5};
    CHECK(arr.data().front() == 1);
    CHECK(arr.data().back() == 5);
}

TEST_CASE("Test at function") {
    Darray<int> arr = {1, 2, 3, 4, 5};

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
    Darray<int> arr;
    arr.reserve(10);

    SUBCASE("Capacity increase") {
        CHECK(arr.capacity() == 10);
    }

    SUBCASE("Capacity unchanged if new_capacity is less than or equal to size") {
        arr.reserve(2);
        CHECK(arr.capacity() == 10);
    }
}