#include "document.h"

Document::Document(int id_val, double relevance_val, int rating_val)
    : id(id_val), relevance(relevance_val), rating(rating_val) {}

std::ostream& operator<<(std::ostream& out, const Document& document) {
    out << "{ "
        << "document_id = " << document.id << ", "
        << "relevance = " << document.relevance << ", "
        << "rating = " << document.rating << " }";

    return out;
}
