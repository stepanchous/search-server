#include "search_server.h"

#include <algorithm>
#include <cmath>
#include <execution>
#include <numeric>
#include <string_view>

#include "string_processing.h"

using namespace std;

SearchServer::SearchServer(const string& stop_words_str)
    : SearchServer(SplitIntoWords(stop_words_str)) {}

SearchServer::SearchServer(const string_view stop_words_sv)
    : SearchServer(SplitIntoWords(stop_words_sv)) {}

set<int>::iterator SearchServer::begin() const { return document_ids_.begin(); }

set<int>::iterator SearchServer::end() const { return document_ids_.end(); }

size_t SearchServer::GetDocumentCount() const { return documents_data_.size(); }

const map<string_view, double>& SearchServer::GetWordFrequencies(
    int document_id) const {
    if (document_ids_.count(document_id) == 0) {
        static map<string_view, double> dummy_empty_map;
        return dummy_empty_map;
    }

    return document_to_word_freqs_.at(document_id);
}

void SearchServer::AddDocument(int document_id, const string_view document_text,
                               DocumentStatus status,
                               const vector<int>& ratings) {
    if (document_id < 0) {
        throw invalid_argument("Id can take only none-negative values"s);
    }
    if (documents_data_.count(document_id) != 0) {
        throw invalid_argument("Document with this id already exist"s);
    }
    if (!IsValidChars(document_text)) {
        throw invalid_argument("Document contents contain invalid characters"s);
    }

    document_ids_.insert(document_id);
    documents_data_[document_id] = {ComputeAverageRating(ratings), status};
    auto text = all_texts_.insert(all_texts_.end(), string(document_text));
    const vector<string_view> document_words = SplitIntoWordsNoStop(*text);
    document_to_word_freqs_[document_id];

    double inverse_words_count = 1.0 / document_words.size();
    for (const string_view word : document_words) {
        word_to_document_freqs_[word][document_id] += inverse_words_count;
        document_to_word_freqs_.at(document_id)[word] += inverse_words_count;
    }
}

void SearchServer::RemoveDocument(int document_id) {
    if (document_ids_.count(document_id) == 0) {
        return;
    }

    for (const auto& [word, _] : document_to_word_freqs_.at(document_id)) {
        word_to_document_freqs_.at(word).erase(document_id);
        if (word_to_document_freqs_.at(word).empty()) {
            word_to_document_freqs_.erase(word);
        }
    }

    document_ids_.erase(document_id);
    document_to_word_freqs_.erase(document_id);
    documents_data_.erase(document_id);
}

tuple<vector<string_view>, DocumentStatus> SearchServer::MatchDocument(
    const string_view raw_query, int document_id) const {
    if (document_ids_.count(document_id) == 0) {
        return {};
    }

    const Query query = ParseQuery(raw_query);
    for (const string_view word : query.minus_words) {
        if (document_to_word_freqs_.at(document_id).count(word)) {
            return {vector<string_view>(),
                    documents_data_.at(document_id).status};
        }
    }

    vector<string_view> matched_words;
    for (const string_view word : query.plus_words) {
        if (document_to_word_freqs_.at(document_id).count(word)) {
            matched_words.push_back(
                document_to_word_freqs_.at(document_id).find(word)->first);
        }
    }
    sort(matched_words.begin(), matched_words.end());

    return {matched_words, documents_data_.at(document_id).status};
}

tuple<vector<string_view>, DocumentStatus> SearchServer::MatchDocument(
    [[maybe_unused]] const execution::sequenced_policy&,
    const string_view raw_query, int document_id) const {
    return MatchDocument(raw_query, document_id);
}

tuple<vector<string_view>, DocumentStatus> SearchServer::MatchDocument(
    const execution::parallel_policy& policy, const string_view raw_query,
    int document_id) const {
    if (document_ids_.count(document_id) == 0) {
        return {};
    }

    const Query query = ParseQuery(raw_query, true);
    const map<string_view, double>& word_to_freq =
        document_to_word_freqs_.at(document_id);
    if (any_of(policy, query.minus_words.begin(), query.minus_words.end(),
               [&](const string_view word) -> bool {
                   return word_to_freq.count(word);
               })) {
        return {vector<string_view>(), documents_data_.at(document_id).status};
    }

    vector<string_view> matched_words;
    matched_words.resize(query.plus_words.size());
    auto matched_words_end = copy_if(
        policy, query.plus_words.begin(), query.plus_words.end(),
        matched_words.begin(),
        [&](const string_view word) { return word_to_freq.count(word); });
    matched_words.resize(matched_words_end - matched_words.begin());
    sort(policy, matched_words.begin(), matched_words.end());
    matched_words.erase(
        unique(policy, matched_words.begin(), matched_words.end()),
        matched_words.end());

    return {matched_words, documents_data_.at(document_id).status};
}

vector<Document> SearchServer::FindTopDocuments(
    const string_view raw_query, DocumentStatus filter_status) const {
    return FindTopDocuments(execution::seq, raw_query, filter_status);
}

double SearchServer::ComputeWordInverseDocumentFreq(
    const string_view word) const {
    return log(static_cast<double>(documents_data_.size()) /
               word_to_document_freqs_.at(word).size());
}

int SearchServer::ComputeAverageRating(const vector<int>& ratings) {
    if (ratings.empty()) {
        return 0;
    }

    return accumulate(ratings.begin(), ratings.end(), 0) /
           static_cast<int>(ratings.size());
}

SearchServer::Query SearchServer::ParseQuery(const string_view text,
                                             bool parallel) const {
    Query query;
    for (string_view word : SplitIntoWordsNoStop(text)) {
        if (!IsValidChars(word)) {
            throw invalid_argument("Query contains invalid characters"s);
        }

        if (word[0] == '-') {
            query.minus_words.push_back(ParseMinusWord(word));
        } else {
            query.plus_words.push_back(word);
        }
    }

    if (!parallel) {
        RemoveDuplicates(query.plus_words);
        RemoveDuplicates(query.minus_words);
    }

    return query;
}

vector<string_view> SearchServer::SplitIntoWordsNoStop(
    const string_view text) const {
    vector<string_view> words;
    for (const string_view word : SplitIntoWords(text)) {
        if (stop_words_.count(word) == 0) {
            words.push_back(word);
        }
    }

    return words;
}

string_view SearchServer::ParseMinusWord(const string_view word) {
    string_view minus_word = word.substr(1);
    if (!IsValidMinusWord(minus_word)) {
        throw invalid_argument("Invalid minus words"s);
    }

    return minus_word;
}

bool SearchServer::IsValidMinusWord(const string_view word) {
    return !(word.empty() || (word[0] == '-'));
}

bool SearchServer::IsValidChars(const string_view word) {
    return none_of(word.begin(), word.end(),
                   [](char c) { return c >= '\0' && c < ' '; });
}
