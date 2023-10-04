#include "process_queries.h"

#include <algorithm>
#include <execution>
#include <string>

#include "search_server.h"

std::vector<SearchResult> ProcessQueries(
    const SearchServer& search_server,
    const std::vector<std::string>& queries) {
    std::vector<SearchResult> search_results(queries.size());
    std::transform(std::execution::par, queries.begin(), queries.end(),
                   search_results.begin(),
                   [&search_server](const std::string& query) {
                       return search_server.FindTopDocuments(query);
                   });

    return search_results;
}

std::deque<Document> ProcessQueriesJoined(
    const SearchServer& search_server,
    const std::vector<std::string>& queries) {
    std::deque<Document> search_results_linear;

    std::vector<SearchResult> search_results_non_linear =
        ProcessQueries(search_server, queries);
    for (const SearchResult& search_result : search_results_non_linear) {
        for (const Document& document : search_result) {
            search_results_linear.push_back(std::move(document));
        }
    }

    return search_results_linear;
}
