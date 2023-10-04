#pragma once

#include <algorithm>
#include <execution>
#include <map>
#include <set>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

#include "concurrent_map.h"
#include "document.h"

constexpr const size_t MAX_RESULT_DOCUMENT_COUNT = 5;

class SearchServer {
   public:
    template <typename Collection>
    explicit SearchServer(const Collection& stop_words);

    explicit SearchServer(const std::string& stop_words_str);

    explicit SearchServer(const std::string_view stop_words_sv);

    std::set<int>::iterator begin() const;

    std::set<int>::iterator end() const;

    const std::map<std::string_view, double>& GetWordFrequencies(
        int document_id) const;

    size_t GetDocumentCount() const;

    void AddDocument(int document_id, const std::string_view document_text,
                     DocumentStatus status, const std::vector<int>& ratings);

    void RemoveDocument(int document_id);

    template <typename ExecutionPolicy>
    void RemoveDocument(ExecutionPolicy&& policy, int document_id);

    std::tuple<std::vector<std::string_view>, DocumentStatus> MatchDocument(
        const std::string_view raw_query, int document_id) const;

    std::tuple<std::vector<std::string_view>, DocumentStatus> MatchDocument(
        const std::execution::sequenced_policy& policy,
        const std::string_view raw_query, int document_id) const;

    std::tuple<std::vector<std::string_view>, DocumentStatus> MatchDocument(
        const std::execution::parallel_policy& policy,
        const std::string_view raw_query, int document_id) const;

    template <typename ExecutionPolicy, typename DocumentPredicate>
    std::vector<Document> FindTopDocuments(
        ExecutionPolicy&& policy, const std::string_view raw_query,
        DocumentPredicate document_predicate) const;

    template <typename DocumentPredicate>
    std::vector<Document> FindTopDocuments(
        const std::string_view raw_query,
        DocumentPredicate document_predicate) const;

    template <typename ExecutionPolicy>
    std::vector<Document> FindTopDocuments(
        ExecutionPolicy&& policy, const std::string_view raw_query,
        DocumentStatus filter_status = DocumentStatus::ACTUAL) const;

    std::vector<Document> FindTopDocuments(
        const std::string_view raw_query,
        DocumentStatus filter_status = DocumentStatus::ACTUAL) const;

   private:
    struct Query {
        std::vector<std::string_view> plus_words;
        std::vector<std::string_view> minus_words;
    };

    struct DocumentData {
        int rating;
        DocumentStatus status;
    };

    std::set<std::string, std::less<>> stop_words_;
    std::set<int> document_ids_;
    std::map<int, DocumentData> documents_data_;
    std::map<std::string_view, std::map<int, double>> word_to_document_freqs_;
    std::map<int, std::map<std::string_view, double>> document_to_word_freqs_;
    std::vector<std::string> all_texts_;

    template <typename ExecutionPolicy, typename DocumentPredicate>
    std::vector<Document> FindAllDocuments(
        ExecutionPolicy&& policy, const Query& query,
        DocumentPredicate document_predicate) const;

    template <typename ExecutionPolicy>
    void RemoveDocumentsWithMinusWords(
        ExecutionPolicy&& policy,
        ConcurrentMap<int, double>& document_to_relevance,
        const Query& query) const;

    template <typename ExecutionPolicy, typename DocumentPredicate>
    ConcurrentMap<int, double> CalculateDocumentsRelevance(
        ExecutionPolicy&& policy, const Query& query,
        DocumentPredicate document_predicate) const;

    double ComputeWordInverseDocumentFreq(const std::string_view word) const;

    Query ParseQuery(const std::string_view text, bool parallel = false) const;

    std::vector<std::string_view> SplitIntoWordsNoStop(
        const std::string_view text) const;

    static int ComputeAverageRating(const std::vector<int>& ratings);

    static std::string_view ParseMinusWord(const std::string_view word);

    static bool IsValidMinusWord(const std::string_view word);

    static bool IsValidChars(const std::string_view word);

    template <typename Collection>
    static std::set<std::string, std::less<>> InitStopWords(
        const Collection& stop_words);

    template <typename T>
    static void RemoveDuplicates(std::vector<T>& vec);
};

template <typename Collection>
SearchServer::SearchServer(const Collection& stop_words)
    : stop_words_(InitStopWords(stop_words)) {
    using namespace std::string_literals;

    if (!all_of(stop_words.begin(), stop_words.end(), IsValidChars)) {
        throw std::invalid_argument("Stop words contain invalid characters"s);
    }
}

template <typename ExecutionPolicy>
void SearchServer::RemoveDocument(ExecutionPolicy&& policy, int document_id) {
    if (document_ids_.count(document_id) == 0) {
        return;
    }

    std::vector<std::string_view> document_words;
    document_words.reserve(document_to_word_freqs_.at(document_id).size());
    std::for_each(
        policy, document_to_word_freqs_.at(document_id).begin(),
        document_to_word_freqs_.at(document_id).end(),
        [&](const std::string_view word) { document_words.push_back(word); });

    std::for_each(policy, document_words.begin(), document_words.end(),
                  [&](const std::string_view word) -> void {
                      word_to_document_freqs_.at(word).erase(document_id);
                  });

    document_ids_.erase(document_id);
    document_to_word_freqs_.erase(document_id);
    documents_data_.erase(document_id);
}

template <typename ExecutionPolicy, typename DocumentPredicate>
std::vector<Document> SearchServer::FindTopDocuments(
    ExecutionPolicy&& policy, const std::string_view raw_query,
    DocumentPredicate document_predicate) const {
    std::vector<Document> matched_documents =
        FindAllDocuments(policy, ParseQuery(raw_query), document_predicate);
    sort(policy, matched_documents.begin(), matched_documents.end(),
         [](const Document& lhs, const Document& rhs) -> bool {
             const double EPS = 10e-6;
             if (std::abs(lhs.relevance - rhs.relevance) < EPS) {
                 return lhs.rating > rhs.rating;
             } else {
                 return lhs.relevance > rhs.relevance;
             }
         });

    if (matched_documents.size() > MAX_RESULT_DOCUMENT_COUNT) {
        matched_documents.resize(MAX_RESULT_DOCUMENT_COUNT);
    }

    return matched_documents;
}

template <typename DocumentPredicate>
std::vector<Document> SearchServer::FindTopDocuments(
    const std::string_view raw_query,
    DocumentPredicate document_predicate) const {
    return FindTopDocuments(std::execution::seq, raw_query, document_predicate);
}

template <typename ExecutionPolicy>
std::vector<Document> SearchServer::FindTopDocuments(
    ExecutionPolicy&& policy, const std::string_view raw_query,
    DocumentStatus filter_status) const {
    auto document_predicate = [filter_status](
                                  [[maybe_unused]] const int id,
                                  [[maybe_unused]] const DocumentStatus status,
                                  [[maybe_unused]] const int rating) {
        return status == filter_status;
    };
    return FindTopDocuments(policy, raw_query, document_predicate);
}

template <typename ExecutionPolicy, typename DocumentPredicate>
std::vector<Document> SearchServer::FindAllDocuments(
    ExecutionPolicy&& policy, const Query& query,
    DocumentPredicate document_predicate) const {
    std::vector<Document> relevant_documents;

    ConcurrentMap<int, double> document_to_relevance =
        CalculateDocumentsRelevance(policy, query, document_predicate);
    RemoveDocumentsWithMinusWords(policy, document_to_relevance, query);
    std::map<int, double> document_to_relevance_map =
        document_to_relevance.BuildMap();
    relevant_documents.reserve(document_to_relevance_map.size());
    for (const auto& [id, relevance] : document_to_relevance_map) {
        relevant_documents.push_back(
            Document(id, relevance, documents_data_.at(id).rating));
    }

    return relevant_documents;
}

template <typename ExecutionPolicy, typename DocumentPredicate>
ConcurrentMap<int, double> SearchServer::CalculateDocumentsRelevance(
    ExecutionPolicy&& policy, const Query& query,
    DocumentPredicate document_predicate) const {
    ConcurrentMap<int, double> document_to_relevance;
    for_each(
        policy, query.plus_words.begin(), query.plus_words.end(),
        [&](const std::string_view word) {
            if (word_to_document_freqs_.count(word) == 0) {
                return;
            }
            double word_idf = ComputeWordInverseDocumentFreq(word);
            for (const auto& [id, word_tf] : word_to_document_freqs_.at(word)) {
                const DocumentData& document = documents_data_.at(id);
                if (document_predicate(id, document.status, document.rating)) {
                    document_to_relevance[id].ref_to_value +=
                        word_idf * word_tf;
                }
            }
        });

    return document_to_relevance;
}

template <typename ExecutionPolicy>
void SearchServer::RemoveDocumentsWithMinusWords(
    ExecutionPolicy&& policy, ConcurrentMap<int, double>& document_to_relevance,
    const Query& query) const {
    for_each(policy, query.minus_words.begin(), query.minus_words.end(),
             [&](const std::string_view word) {
                 if (word_to_document_freqs_.count(word) == 0) {
                     return;
                 }

                 for (const auto& [document_id, _] :
                      word_to_document_freqs_.at(word)) {
                     document_to_relevance.erase(document_id);
                 }
             });
}

template <typename Collection>
std::set<std::string, std::less<>> SearchServer::InitStopWords(
    const Collection& stop_words) {
    std::set<std::string, std::less<>> stop_words_set;
    for (const std::string_view word : stop_words) {
        if (!word.empty()) {
            stop_words_set.insert(std::string(word));
        }
    }

    return stop_words_set;
}

template <typename T>
void SearchServer::RemoveDuplicates(std::vector<T>& vec) {
    std::sort(vec.begin(), vec.end());
    vec.erase(std::unique(vec.begin(), vec.end()), vec.end());
}
