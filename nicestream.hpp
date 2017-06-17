/*
    Copyright 2017 LIPKA Boldizsár

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to
    deal in the Software without restriction, including without limitation the
    rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
    sell copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in
    all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
    FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
    IN THE SOFTWARE.
*/

#ifndef NICESTREAM_HPP_INCLUDED
#define NICESTREAM_HPP_INCLUDED

#include <string>
#include <iostream>

namespace nice {

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

