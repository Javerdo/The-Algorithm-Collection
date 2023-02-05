#include <algorithm/darray.h>
#include <iostream>

int main(int argc, char** argv) {

    Darray<int> arr = { 1, 2, 3, 4, 5 };

    std::cout << "Darray size: " << arr.size() << std::endl;

    std::cout << "Darray elements: ";
    for (const auto& elem : arr) {
        std::cout << elem << " ";
    }
    std::cout << std::endl;

    arr.push_back(6);
    std::cout << "Darray after push_back(6): ";
    for (const auto& elem : arr) {
        std::cout << elem << " ";
    }
    std::cout << std::endl;

    arr.pop_back();
    std::cout << "Darray after pop_back(): ";
    for (const auto& elem : arr) {
        std::cout << elem << " ";
    }
    std::cout << std::endl;

    return 0;
}
