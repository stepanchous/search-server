// -------- Начало модульных тестов поисковой системы ----------
#pragma GCC diagnostic ignored "-Wunused-parameter"

#include "../search_server.h"
#include "test-framework.h"

void TestExcludeStopWordsFromAddedDocumentContent() {
    SearchServer server("a"s);

    server.AddDocument(1, "a b", DocumentStatus::ACTUAL, {1, 2});
    server.AddDocument(2, "a c", DocumentStatus::ACTUAL, {1, 2});
    auto res = server.GetWordFrequencies(1);
    ASSERT_EQUAL(res.count("a"), 0);
}

void TestExcludeDocumentsWithMinusWords() {
    const int doc_id = 42;
    const string content = "cat in the city"s;
    const vector<int> ratings = {1, 2, 3};

    {
        SearchServer server(""s);
        server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        const auto found_docs = server.FindTopDocuments("cat"s);
        ASSERT_EQUAL(found_docs.size(), 1);
        const Document& doc0 = found_docs[0];
        ASSERT_EQUAL(doc0.id, doc_id);
    }

    {
        SearchServer server(""s);
        server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        ASSERT(server.FindTopDocuments("cat -in"s).empty());
    }
}

void TestDocumentMatch() {
    const int doc_id = 42;
    const string content = "cat in the city"s;
    const vector<int> ratings = {1, 2, 3};

    {
        SearchServer server(""s);
        server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        // document matches
        {
            const auto [words, status] = server.MatchDocument("in cat"s, 42);
            const vector<string_view> expected_result = {"cat"sv, "in"sv};
            ASSERT_EQUAL(words, expected_result);
        }

        // document contains minus-words
        {
            const auto [words, status] = server.MatchDocument("-in cat"s, 42);
            ASSERT(words.empty());
        }
    }
}

void TestRelevanceSortInFindTopDocuments() {
    const int id_1 = 1;
    const string content_1 = "a b c"s;
    const vector<int> ratings_1 = {1, 2, 3};
    const int id_2 = 2;
    const string content_2 = "a b"s;
    const vector<int> ratings_2 = {1, 2, 3};
    {
        SearchServer server(""s);
        {  // check empty result
            const vector<Document> result = server.FindTopDocuments("a b c"s);
            ASSERT(result.empty());
        }

        {
            server.AddDocument(id_1, content_1, DocumentStatus::ACTUAL,
                               ratings_1);
            server.AddDocument(id_2, content_2, DocumentStatus::ACTUAL,
                               ratings_2);
            const vector<Document> result = server.FindTopDocuments("a b c"s);
            ASSERT_EQUAL(result[0].id, id_1);
            ASSERT_EQUAL(result[1].id, id_2);
        }
    }
}

void TestAverageRatingCalculation() {
    const int doc_id = 42;
    const string content = "cat in the city"s;

    {
        SearchServer server(""s);
        vector<int> ratings = {};
        server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        const vector<Document> result = server.FindTopDocuments("cat"s);
        ASSERT_EQUAL(result[0].rating, 0);
    }

    {
        SearchServer server(""s);
        vector<int> ratings = {1, 2, 3};
        server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        const vector<Document> result = server.FindTopDocuments("cat"s);
        ASSERT_EQUAL(result[0].rating, 2);
    }
}

void TestPredicateFilter() {
    const int id_1 = 1;
    const string content_1 = "a b c"s;
    const vector<int> ratings_1 = {1, 2, 3};
    const int id_2 = 2;
    const string content_2 = "a b"s;
    const vector<int> ratings_2 = {1, 2, 3};
    {
        SearchServer server(""s);
        {
            server.AddDocument(id_1, content_1, DocumentStatus::ACTUAL,
                               ratings_1);
            server.AddDocument(id_2, content_2, DocumentStatus::ACTUAL,
                               ratings_2);
            const vector<Document> result = server.FindTopDocuments(
                "a b c"s, [](const int id, const DocumentStatus status,
                             const int rating) { return id % 2 == 0; });
            for (const auto& res : result) {
                ASSERT_EQUAL(res.id % 2, 0);
            }
        }
    }
}

void TestStatusFilter() {
    const int id_1 = 1;
    const string content_1 = "a b c"s;
    const vector<int> ratings_1 = {1, 2, 3};
    const int id_2 = 2;
    const string content_2 = "a b"s;
    const vector<int> ratings_2 = {1, 2, 3};
    {
        SearchServer server(""s);
        {
            server.AddDocument(id_1, content_1, DocumentStatus::IRRELEVANT,
                               ratings_1);
            server.AddDocument(id_2, content_2, DocumentStatus::ACTUAL,
                               ratings_2);
            const vector<Document> result =
                server.FindTopDocuments("a b c"s, DocumentStatus::IRRELEVANT);
            ASSERT_EQUAL(result[0].id, id_1);
        }
    }
}

void TestRelevanceCalculation() {
    const int id_1 = 1;
    const string content_1 = "a b c"s;
    const vector<int> ratings_1 = {1, 2, 3};
    const int id_2 = 2;
    const string content_2 = "a b"s;
    const vector<int> ratings_2 = {1, 2, 3};

    {
        SearchServer server(""s);
        {
            server.AddDocument(id_1, content_1, DocumentStatus::ACTUAL,
                               ratings_1);
            server.AddDocument(id_2, content_2, DocumentStatus::ACTUAL,
                               ratings_2);
            const vector<Document> result = server.FindTopDocuments("a b c"s);
            ASSERT(result[0].relevance > 0.231 && result[0].relevance < 0.232);
            ASSERT_EQUAL(result[1].relevance, 0.0);
        }
    }
}

const std::vector<int> ratings1 = {1, 2, 3, 4, 5};
const std::vector<int> ratings2 = {-1, -2, 30, -3, 44, 5};
const std::vector<int> ratings3 = {12, -20, 80, 0, 8, 0, 0, 9, 67};
const std::vector<int> ratings4 = {7, 0, 3, -49, 5};
const std::vector<int> ratings5 = {81, -6, 7, 94, -7};
const std::vector<int> ratings6 = {41, 8, -7, 897, 5};
const std::vector<int> ratings7 = {543, 0, 43, 4, -5};
const std::vector<int> ratings8 = {91, 7, 3, -88, 56};
const std::vector<int> ratings9 = {0, -87, 93, 66, 5};
const std::vector<int> ratings10 = {11, 2, -43, 4, 895};
void PrintDocumentUTest(const Document& document) {
    std::cout << "{ "
              << "document_id = " << document.id << ", "
              << "relevance = " << document.relevance << ", "
              << "rating = " << document.rating << " }" << std::endl;
}
void TestRating() {
    std::string stop_words = "и в на";
    SearchServer lambda_search_server(stop_words);

    lambda_search_server.AddDocument(0, "белый кот и модный ошейник",
                                     DocumentStatus::ACTUAL, ratings1);
    lambda_search_server.AddDocument(1, "пушистый кот пушистый хвост",
                                     DocumentStatus::ACTUAL, ratings2);
    lambda_search_server.AddDocument(2, "ухоженный пёс выразительные глаза",
                                     DocumentStatus::ACTUAL, ratings3);
    lambda_search_server.AddDocument(3, "белый модный кот",
                                     DocumentStatus::IRRELEVANT, ratings4);
    lambda_search_server.AddDocument(4, "пушистый кот пёс",
                                     DocumentStatus::IRRELEVANT, ratings5);
    lambda_search_server.AddDocument(5, "ухоженный ошейник выразительные глаза",
                                     DocumentStatus::IRRELEVANT, ratings6);
    lambda_search_server.AddDocument(6, "кот и ошейник", DocumentStatus::BANNED,
                                     ratings7);
    lambda_search_server.AddDocument(7, "пёс и хвост", DocumentStatus::BANNED,
                                     ratings8);
    lambda_search_server.AddDocument(8, "модный пёс пушистый хвост",
                                     DocumentStatus::BANNED, ratings9);
    lambda_search_server.AddDocument(9, "кот пушистый ошейник",
                                     DocumentStatus::REMOVED, ratings10);
    lambda_search_server.AddDocument(10, "ухоженный кот и пёс",
                                     DocumentStatus::REMOVED, ratings2);
    lambda_search_server.AddDocument(11, "хвост и выразительные глаза",
                                     DocumentStatus::REMOVED, ratings3);

    const std::string lambda_query = "пушистый ухоженный кот";
    cout << "Ratings > 10:" << endl;
    const auto documentsRatingBigger10 = lambda_search_server.FindTopDocuments(
        lambda_query, [](int document_id, DocumentStatus status, int rating) {
            return rating > 10;
        });
    for (const Document& document : documentsRatingBigger10) {
        PrintDocumentUTest(document);
    }
    cout << "Even ratings:" << endl;
    const auto documentsEvenRating = lambda_search_server.FindTopDocuments(
        lambda_query, [](int document_id, DocumentStatus status, int rating) {
            return rating % 2 == 0;
        });
    for (const Document& document : documentsEvenRating) {
        PrintDocumentUTest(document);
    }
}

void TestGetWordFrequencies() { SearchServer search_server(""s); }

void TestSearchServer() {
    RUN_TEST(TestExcludeStopWordsFromAddedDocumentContent);
    RUN_TEST(TestExcludeDocumentsWithMinusWords);
    RUN_TEST(TestDocumentMatch);
    RUN_TEST(TestRelevanceSortInFindTopDocuments);
    RUN_TEST(TestAverageRatingCalculation);
    RUN_TEST(TestPredicateFilter);
    RUN_TEST(TestStatusFilter);
    RUN_TEST(TestRelevanceCalculation);
    RUN_TEST(TestRating);
}

int main() {
    TestSearchServer();

    return 0;
}

// --------- Окончание модульных тестов поисковой системы -----------
