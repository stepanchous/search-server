#include "remove_duplicates.h"

using namespace std;

void RemoveDuplicates(SearchServer& search_server) {
    set<set<string_view, less<>>> unique_words_sets;

    set<int> remove_ids;
    for (int document_id : search_server) {
        set<string_view, less<>> words_set =
            GetKeys(search_server.GetWordFrequencies(document_id));
        if (unique_words_sets.count(words_set) == 1) {
            remove_ids.insert(document_id);
        }

        unique_words_sets.insert(words_set);
    }

    for (int id : remove_ids) {
        search_server.RemoveDocument(id);
        cout << "Found duplicate document id " << id << endl;
    }
}
