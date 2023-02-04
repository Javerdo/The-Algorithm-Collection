#include <algorithm/array.h>
#include <iostream>

int main(int argc, char** argv) {

    Array<int> arr = { 1, 2, 3, 4, 5 };

    std::cout << "Array size: " << arr.size() << std::endl;

    std::cout << "Array elements: ";
    for (const auto& elem : arr) {
        std::cout << elem << " ";
    }
    std::cout << std::endl;

    arr.push_back(6);
    std::cout << "Array after push_back(6): ";
    for (const auto& elem : arr) {
        std::cout << elem << " ";
    }
    std::cout << std::endl;

    arr.pop_back();
    std::cout << "Array after pop_back(): ";
    for (const auto& elem : arr) {
        std::cout << elem << " ";
    }
    std::cout << std::endl;

    return 0;
  return 0;
}
