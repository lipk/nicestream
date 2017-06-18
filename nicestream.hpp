#ifndef NICESTREAM_HPP_INCLUDED
#define NICESTREAM_HPP_INCLUDED

#include <string>
#include <iostream>

namespace nstr {

class nfa;
class sep {
    friend std::istream& operator >>(std::istream&, sep);
    nfa* nfa_p;
public:
    sep(const std::string& regex);
    sep(const sep&);
    sep(const sep&&);
    sep& operator =(const sep&);
    sep& operator =(sep&&);
    ~sep();
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

struct invalid_regex : public std::exception {};
struct invalid_input : public std::exception {};
struct stream_error : public std::exception {};

}

#endif

