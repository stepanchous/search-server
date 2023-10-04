#pragma once

#include <iostream>
#include <vector>

template <typename Iterator>
class IteratorRange {
   public:
    explicit IteratorRange(Iterator& begin, Iterator& end)
        : begin_(begin), end_(end) {}

    Iterator begin() { return begin_; }

    Iterator end() { return end_; }

    Iterator size() { return distance(begin_, end_); }

   private:
    Iterator begin_;
    Iterator end_;
};

template <typename Iterator>
class Paginator {
   public:
    Paginator(Iterator begin, Iterator end, size_t page_size) {
        while (begin != end) {
            Iterator range_begin = begin;
            size_t advance_range = static_cast<size_t>(distance(begin, end));
            advance(begin, std::min(page_size, advance_range));
            Iterator range_end = begin;
            pages_.push_back(IteratorRange<Iterator>(range_begin, range_end));
        }
    }

    auto begin() const { return pages_.begin(); }

    auto end() const { return pages_.end(); }

   private:
    std::vector<IteratorRange<Iterator>> pages_;
};

template <typename Container>
auto Paginate(const Container& c, size_t page_size) {
    return Paginator(begin(c), end(c), page_size);
}

template <typename T>
std::ostream& operator<<(std::ostream& out, IteratorRange<T> iter_range) {
    T iter_begin = iter_range.begin();
    T iter_end = iter_range.end();
    while (iter_begin != iter_end) {
        out << *iter_begin;
        ++iter_begin;
    }

    return out;
}
