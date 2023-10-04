#pragma once
#pragma GCC diagnostic ignored "-Wsign-compare"

#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <vector>

using namespace std;

template <typename T1, typename T2>
ostream& operator<<(ostream& out, const pair<T1, T2>& p) {
    cerr << p.first << ": " << p.second;
    return out;
}

template <typename Container>
void Print(ostream& out, const Container& container) {
    bool is_first = true;
    for (const auto& element : container) {
        if (is_first) {
            out << element;
            is_first = false;
        } else {
            out << ", " << element;
        }
    }
}

template <typename T>
ostream& operator<<(ostream& out, const vector<T>& container) {
    cerr << "[";
    Print(out, container);
    cerr << "]";
    return out;
}

template <typename T>
ostream& operator<<(ostream& out, const set<T>& container) {
    cerr << "{";
    Print(out, container);
    cerr << "}";
    return out;
}

template <typename T1, typename T2>
ostream& operator<<(ostream& out, const map<T1, T2>& container) {
    cerr << "{";
    Print(out, container);
    cerr << "}";
    return out;
}

void AssertImpl(bool value, const string& expr_str, const string& file,
                const string& func, unsigned line, const string& hint) {
    if (!value) {
        cerr << file << "(" << line << "): " << func << ": ";
        cerr << "ASSERT(" << expr_str << ") failed.";
        if (!hint.empty()) {
            cerr << " Hint: " << hint;
        }
        cerr << endl;

        abort();
    }
}

#define ASSERT(expr) AssertImpl((expr), #expr, __FILE__, __FUNCTION__, __LINE__, ""s)
#define ASSERT_HINT(expr, hint) AssertImpl((expr), #expr, __FILE__, __FUNCTION__, __LINE__, (hint))

template <typename T, typename U>
void AssertEqualImpl(const T& t, const U& u, const string& t_str, const string& u_str, const string& file,
                     const string& func, unsigned line, const string& hint) {
    if (t != u) {
        cerr << boolalpha;
        cerr << file << "("s << line << "): "s << func << ": "s;
        cerr << "ASSERT_EQUAL("s << t_str << ", "s << u_str << ") failed: "s;
        cerr << t << " != "s << u << "."s;
        if (!hint.empty()) {
            cerr << " Hint: "s << hint;
        }
        cerr << endl;
        abort();
    }
}

#define ASSERT_EQUAL(a, b) AssertEqualImpl((a), (b), #a, #b, __FILE__, __FUNCTION__, __LINE__, ""s)
#define ASSERT_EQUAL_HINT(a, b, hint) AssertEqualImpl((a), (b), #a, #b, __FILE__, __FUNCTION__, __LINE__, (hint))

template <typename Test>
void RunTest(Test test, const string& test_name) {
    test();
    cerr << test_name << " OK" << endl;
}

#define RUN_TEST(test) RunTest((test), #test)