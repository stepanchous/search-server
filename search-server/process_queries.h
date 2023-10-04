#pragma once

#include <deque>
#include <string>
#include <vector>

#include "document.h"
#include "search_server.h"

typedef std::vector<Document> SearchResult;

std::vector<SearchResult> ProcessQueries(
    const SearchServer& search_server, const std::vector<std::string>& queries);

std::deque<Document> ProcessQueriesJoined(
    const SearchServer& search_server, const std::vector<std::string>& queries);
