#ifndef NICESTREAM_HPP_INCLUDED
#define NICESTREAM_HPP_INCLUDED

#include <string>
#include <iostream>
#include "nfa.hpp"

namespace nstr {

class sep {
    friend std::istream& operator >>(std::istream&, sep);
    nstr_private::nfa_executor executor;
public:
    sep(const std::string& regex);
    sep(const sep&);
    sep(const sep&&);
    sep& operator =(const sep&);
    sep& operator =(sep&&);
};

std::istream& operator >>(std::istream& is, sep field);

template<typename ...Fields>
class skip {};

template<typename First, typename ...Rest>
std::istream& operator >>(std::istream& is, skip<First, Rest...>) {
    First f;
    is >> f;
    return is >> skip<Rest...>();
}

std::istream& operator >>(std::istream& is, skip<>);

class until {
    friend std::istream& operator >>(std::istream&, until);
    nstr_private::nfa_executor nfa;
    std::string &dst;
public:
    until(const std::string& regex, std::string &dst);
};

std::istream &operator >>(std::istream &is, until obj);

class all {
    friend std::istream& operator >>(std::istream&, all);
    std::string &dst;
public:
    all(std::string &dst);
};

std::istream &operator >>(std::istream &is, all obj);

struct invalid_input : public std::exception {};
struct stream_error : public std::exception {};
struct invalid_regex : public std::exception {};
}

#endif

