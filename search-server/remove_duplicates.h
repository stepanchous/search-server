#pragma once

#include <algorithm>
#include <map>
#include <set>

#include "search_server.h"

template <typename T, typename U>
std::set<T, std::less<>> GetKeys(const std::map<T, U>& m) {
    std::set<T, std::less<>> key_set;
    transform(m.begin(), m.end(), inserter(key_set, key_set.end()),
              [](auto pair) { return pair.first; });

    return key_set;
}

void RemoveDuplicates(SearchServer& search_server);
