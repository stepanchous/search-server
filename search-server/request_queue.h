#pragma once

#include <deque>
#include <string>
#include <vector>

#include "search_server.h"

class RequestQueue {
   public:
    explicit RequestQueue(const SearchServer& search_server);

    int GetNoResultRequests() const;

    template <typename DocumentPredicate>
    std::vector<Document> AddFindRequest(const std::string& raw_query,
                                         DocumentPredicate document_predicate);

    std::vector<Document> AddFindRequest(const std::string& raw_query,
                                         DocumentStatus status);

    std::vector<Document> AddFindRequest(const std::string& raw_query);

   private:
    struct QueryResult {
        bool empty;
        uint64_t timestamp;

        QueryResult(bool empty, uint64_t time)
            : empty(empty), timestamp(time) {}
    };

    std::deque<QueryResult> requests_;
    const SearchServer& search_server_;
    uint64_t current_time;
    int empty_result_count;
    const static int min_in_day_ = 1440;

    void AddRequest(bool empty);
};

template <typename DocumentPredicate>
std::vector<Document> RequestQueue::AddFindRequest(
    const std::string& raw_query, DocumentPredicate document_predicate) {
    std::vector<Document> matched_documents =
        search_server_.FindTopDocuments(raw_query, document_predicate);
    AddRequest(matched_documents.empty());

    return matched_documents;
}
