#include "request_queue.h"

using namespace std;

RequestQueue::RequestQueue(const SearchServer& search_server)
    : search_server_(search_server), current_time(0), empty_result_count(0) {}

int RequestQueue::GetNoResultRequests() const { return empty_result_count; }

void RequestQueue::AddRequest(bool empty) {
    ++current_time;

    while (!requests_.empty() &&
           (current_time - requests_.back().timestamp) >= min_in_day_) {
        if (requests_.back().empty) {
            --empty_result_count;
        }
        requests_.pop_back();
    }

    requests_.push_front(QueryResult(empty, current_time));
    if (empty) {
        ++empty_result_count;
    }
}

vector<Document> RequestQueue::AddFindRequest(const string& raw_query,
                                              DocumentStatus status) {
    vector<Document> matched_documents =
        search_server_.FindTopDocuments(raw_query, status);
    AddRequest(matched_documents.empty());

    return matched_documents;
}

vector<Document> RequestQueue::AddFindRequest(const string& raw_query) {
    vector<Document> matched_documents =
        search_server_.FindTopDocuments(raw_query);
    AddRequest(matched_documents.empty());

    return matched_documents;
}
