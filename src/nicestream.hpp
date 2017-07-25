#ifndef NICESTREAM_HPP_INCLUDED
#define NICESTREAM_HPP_INCLUDED

#include <string>
#include <iostream>
#include <sstream>
#include "nfa.hpp"

namespace nstr {

struct invalid_input : public std::exception {};
struct stream_error : public std::exception {};
struct invalid_regex : public std::exception {};

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

template<typename ItemT>
class split {
    template <typename T>
    friend std::istream &operator >>(std::istream&, split<T>);
    std::vector<ItemT> &dst;
    nstr_private::nfa_executor nfa_sep;
    nstr_private::nfa_executor nfa_fin;
public:
    split(const std::string &seprx, const std::string &finrx, 
            std::vector<ItemT> &dst);
};

template<typename ItemT>
split<ItemT>::split(const std::string &seprx, const std::string &finrx, std::vector<ItemT> &dst)
    : dst(dst), nfa_sep(seprx), nfa_fin(finrx) {}

template<typename ItemT>
std::istream &operator >>(std::istream &is, split<ItemT> obj) {
    std::string buf;
    bool sep_matched = false;
    size_t match_len = 0, match_start = 0;
    while (true) {
        std::cout << "!! " << buf << std::endl;
        if (is.eof()) {
            throw invalid_input();
        }
        uint8_t sym = is.get();
        obj.nfa_fin.next(sym);
        obj.nfa_fin.start_path();
        obj.nfa_sep.next(sym);
        if (!sep_matched) {
            obj.nfa_sep.start_path();
        } else if (obj.nfa_sep.match() == nstr_private::match_state::ACCEPT) {
            match_len = obj.nfa_sep.longest_match();
        }
        buf.push_back(sym);
        if (obj.nfa_fin.match() == nstr_private::match_state::ACCEPT) {
            break;
        }
        if (!sep_matched && obj.nfa_sep.match() == nstr_private::match_state::ACCEPT) {
            sep_matched = true;
            match_len = obj.nfa_sep.trim_short_matches();
            match_start = buf.size() - match_len;
        } else if (sep_matched && obj.nfa_sep.match() == nstr_private::match_state::REFUSE) {
            for (size_t i = buf.size(); i>match_start+match_len; --i) {
                is.putback(buf.back());
                buf.pop_back();
            }
            buf.resize(buf.size()-match_len);
            obj.dst.emplace_back();
            std::stringstream ss(std::move(buf));
            ss >> obj.dst.back();
            obj.nfa_sep.reset();
            obj.nfa_fin.reset();
            sep_matched = false;
            match_len = 0;
            buf.clear();
        }
    }

    match_len = obj.nfa_fin.trim_short_matches();
    match_start = buf.size()-match_len;
    while (obj.nfa_fin.match() != nstr_private::match_state::REFUSE) {
        uint8_t sym = is.get();
        obj.nfa_fin.next(sym);
        buf.push_back(sym);
        if (obj.nfa_fin.match() == nstr_private::match_state::ACCEPT) {
            match_len = obj.nfa_fin.longest_match();
        }
    }
    for (size_t i = buf.size(); i>match_start+match_len; --i) {
        is.putback(buf.back());
        buf.pop_back();
    }
    buf.resize(buf.size()-match_len);
    obj.dst.emplace_back();
    std::stringstream ss(std::move(buf));
    ss >> obj.dst.back();
    return is;
}
}
#endif

