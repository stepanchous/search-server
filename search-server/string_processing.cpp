#include "string_processing.h"

using namespace std;

vector<string_view> SplitIntoWords(string_view str) {
    vector<string_view> words;

    str.remove_prefix(min(str.size(), str.find_first_not_of(" ")));
    while (!str.empty()) {
        size_t space_pos = str.find(" ");
        words.push_back(str.substr(0, space_pos));
        str.remove_prefix(
            min(str.size(), str.find_first_not_of(" ", space_pos)));
    }

    return words;
}
