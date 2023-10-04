#include "read_input_functions.h"

std::string ReadLine() {
    std::string s;
    std::getline(std::cin, s);

    return s;
}

std::vector<int> ReadIntVector() {
    std::vector<int> vec;
    int element_count;

    std::cin >> element_count;
    vec.reserve(element_count);

    int el;
    for (int i = 0; i < element_count; ++i) {
        std::cin >> el;
        vec.push_back(el);
    }
    ReadLine();

    return vec;
}

int ReadLineWithNumber() {
    int result = 0;
    std::cin >> result;
    ReadLine();

    return result;
}
