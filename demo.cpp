#include "nicestream.hpp"
#include <iostream>

int main() {
    // read three ints; discard the second one
    int i, j;
    std::cin >> i >> nice::skip<int>() >> j;
    std::cout << i << " " << j << std::endl;

    // read two ints separated by a comma
    std::cin >> i >> nice::sep(", *") >> j;
    std::cout << i << " " << j << std::endl;
}

