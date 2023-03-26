#include <doctest/doctest.h>
#include <algorithmCollection/data structures/dynamicArray.h>
#include <iostream>

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

    SUBCASE("Decrease size (size == 4)") {
        arr.pop_back();
        CHECK(arr.size() == 4);
        CHECK(arr[3] == 4);
    }

    SUBCASE("Decrease size (size == 3)") {
        arr.pop_back();
        arr.pop_back();
        CHECK(arr.size() == 3);
        CHECK(arr[2] == 3);
    }
}

TEST_CASE("Test pop_front function") {
    DynamicArray<int> arr = {1, 2, 3, 4, 5};
    SUBCASE("Decrease size (size == 4)") {
        arr.pop_front();
        CHECK(arr.size() == 4);
        CHECK(arr[0] == 2);
    }

    SUBCASE("Decrease size (size == 3)") {
        arr.pop_front();
        arr.pop_front();
        CHECK(arr.size() == 3);
        CHECK(arr[0] == 3);
    }

    SUBCASE("Decrease size (size == 2)") {
        arr.pop_front();
        arr.pop_front();
        arr.pop_front();
        CHECK(arr.size() == 2);
        CHECK(arr[0] == 4);
    }

    SUBCASE("Decrease size (size == 1)") {
        arr.pop_front();
        arr.pop_front();
        arr.pop_front();
        arr.pop_front();
        CHECK(arr.size() == 1);
        CHECK(arr[0] == 5);
    }
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

    SUBCASE("Test removing an element at the middle") {
        auto it1 = arr.begin() + 1;
        arr.erase(it1);
        DynamicArray<int> expected = {1, 3, 4};
        CHECK(arr == expected);
    }

    SUBCASE("Test removing an element at the end") {
        auto it2 = arr.end() - 1;
        arr.erase(it2);
        DynamicArray<int> expected = {1, 2, 3};
        CHECK(arr == expected);
    }

    SUBCASE("Test removing an element at the beginning") {
        auto it3 = arr.begin();
        arr.erase(it3);
        DynamicArray<int> expected = {2, 3, 4};
        CHECK(arr == expected);
    }

    SUBCASE("Test removing the only element in the array") {
        arr.resize(1);
        auto it4 = arr.begin();
        arr.erase(it4);
        DynamicArray<int> expected = {};
        CHECK(arr == expected);
    }

    SUBCASE("Test removing from an empty array") {
        DynamicArray<int> expected;
        auto it5 = expected.begin();
        CHECK_THROWS(expected.erase(it5));
    }
}

TEST_CASE("Test copy assignment operator") {
    DynamicArray<int> arr1 = {1, 2, 3, 4, 5};
    DynamicArray<int> arr2;
    arr2 = arr1;
    CHECK(arr1.size() == arr2.size());
    CHECK(arr1[0] == arr2[0]);
    CHECK(arr1[4] == arr2[4]);
}

TEST_CASE("Test move assignment operator") {
    DynamicArray<int> arr1 = {1, 2, 3, 4, 5};
    DynamicArray<int> arr2;
    arr2 = std::move(arr1);
    CHECK(arr1.empty());
    CHECK(arr2.size() == 5);
    CHECK(arr2[0] == 1);
    CHECK(arr2[4] == 5);
}

TEST_CASE("Test insert function") {
    DynamicArray<int> arr = {1, 2, 4, 5};
    auto it = arr.begin() + 2;
    arr.insert(it, 3);
    CHECK(arr.size() == 5);
    CHECK(arr[0] == 1);
    CHECK(arr[1] == 2);
    CHECK(arr[2] == 3);
    CHECK(arr[3] == 4);
    CHECK(arr[4] == 5);
}

TEST_CASE("Test clear function") {
    DynamicArray<int> arr = {1, 2, 3, 4, 5};
    arr.clear();
    CHECK(arr.empty());
    CHECK(arr.size() == 0);
}

struct CustomObject {
    int id;
    std::string name;
};

TEST_CASE("Test with custom object") {
    DynamicArray<CustomObject> arr;
    arr.push_back({1, "Alice"});
    arr.push_back({2, "Bob"});
    CHECK(arr.size() == 2);
    CHECK(arr[0].id == 1);
    CHECK(arr[0].name == "Alice");
    CHECK(arr[1].id == 2);
    CHECK(arr[1].name == "Bob");
}

TEST_CASE("Test with a large number of elements") {
    DynamicArray<int> arr;
    constexpr int LARGE_NUM = 10000;
    for (int i = 0; i < LARGE_NUM; ++i) {
        arr.push_back(i);
    }
    CHECK(arr.size() == LARGE_NUM);
    CHECK(arr[0] == 0);
    CHECK(arr[LARGE_NUM - 1] == LARGE_NUM - 1);
}

TEST_CASE("Test resize function with one parameter") {
    DynamicArray<int> arr = {1, 2, 3, 4, 5};

    SUBCASE("Increase size") {
        arr.resize(7);
        CHECK(arr.size() == 7);
        CHECK(arr[6] == 0); // Check that the new elements are initialized
    }

    SUBCASE("Decrease size") {
        arr.resize(3);
        CHECK(arr.size() == 3);
        CHECK(arr[2] == 3);
    }

    SUBCASE("Keep size the same") {
        arr.resize(5);
        CHECK(arr.size() == 5);
        CHECK(arr[4] == 5);
    }
}

struct CustomInt {
    int value;

    CustomInt() : value(42) {}
    CustomInt(int v) : value(v) {}

    bool operator==(const CustomInt& other) const {
        return value == other.value;
    }
};

TEST_CASE("Test resize function with custom type") {
    DynamicArray<CustomInt> arr = {1, 2, 3, 4, 5};

    SUBCASE("Increase size") {
        arr.resize(7);
        CHECK(arr.size() == 7);
        CHECK(arr[6] == CustomInt()); // Check that the new elements are initialized with the default value
    }
}

TEST_CASE("Test resize function with two parameters") {
    DynamicArray<int> arr = {1, 2, 3, 4, 5};

    SUBCASE("Increase size with custom value") {
        arr.resize(7, 42);
        CHECK(arr.size() == 7);
        CHECK(arr[4] == 5); // Check that the existing elements are unchanged
        CHECK(arr[5] == 42); // Check that the new elements have the custom value
        CHECK(arr[6] == 42);
    }

    SUBCASE("Decrease size with custom value") {
        arr.resize(3, 42);
        CHECK(arr.size() == 3);
        CHECK(arr[2] == 3);
    }

    SUBCASE("Keep size the same with custom value") {
        arr.resize(5, 42);
        CHECK(arr.size() == 5);
        CHECK(arr[4] == 5);
    }
}


TEST_CASE("Test shrink_to_fit function") {
    DynamicArray<int> arr = {1, 2, 3, 4, 5};
    arr.reserve(10);
    CHECK(arr.capacity() == 10);
    arr.shrink_to_fit();
    CHECK(arr.capacity() == arr.size());
}

TEST_CASE("Test swap function") {
    DynamicArray<int> arr1 = {1, 2, 3};
    DynamicArray<int> arr2 = {4, 5, 6, 7};

    arr1.swap(arr2);

    CHECK(arr1.size() == 4);
    CHECK(arr1[0] == 4);
    CHECK(arr1[3] == 7);

    CHECK(arr2.size() == 3);
    CHECK(arr2[0] == 1);
    CHECK(arr2[2] == 3);
}

TEST_CASE("Test operator[] const and non-const overloads") {
    const DynamicArray<int> const_arr = {1, 2, 3};
    DynamicArray<int> non_const_arr = {1, 2, 3};

    CHECK(const_arr[0] == 1);
    CHECK(non_const_arr[0] == 1);

    non_const_arr[0] = 5;
    CHECK(non_const_arr[0] == 5);
}

TEST_CASE("Test iterator functions") {
    DynamicArray<int> arr = {1, 2, 3, 4, 5};

    SUBCASE("Test begin and end") {
        int expected = 1;
        for (auto it = arr.begin(); it != arr.end(); ++it) {
            CHECK(*it == expected);
            expected++;
        }
    }

    SUBCASE("Test rbegin and rend") {
        int expected = 5;
        for (auto it = arr.rbegin(); it != arr.rend(); ++it) {
            CHECK(*it == expected);
            expected--;
        }
    }
}