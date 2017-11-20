#include "nicein.hpp"
#include <cstdint>
#include <string>
#include <map>
#include <vector>
#include <cctype>

using namespace nstr_private;

namespace nstr {
// *************************************************************
// STREAM STUFF
// *************************************************************

sep::sep(const std::string& regex) : rx(regex, dummy)
{
}

std::istream &operator >>(std::istream &is, sep what) {
    return is >> what.rx;
}

std::istream& operator >>(std::istream& is, skip<>) {
    return is;
}

until::until(const std::string &regex, std::string &dst) 
    : nfa(regex), dst(dst) {}

until::until(const std::string &regex) 
    : nfa(regex), dst(this->dummy) {}

std::istream &operator >>(std::istream &is, until obj) {
    while (obj.nfa.match() != match_state::ACCEPT) {
        if (is.eof()) {
            throw invalid_input();
        }
        uint8_t sym = is.get();
        obj.nfa.next(sym);
        obj.nfa.start_path();
        obj.dst.push_back(sym);
    }
    const size_t len = obj.nfa.trim_short_matches();
    obj.dst.resize(obj.dst.size() - len);
    std::vector<uint8_t> buf;
    while (true) {
        uint8_t next = static_cast<uint8_t>(is.get());
        if (is.eof()) {
            is.clear();
            break;
        }
        buf.push_back(next);
        obj.nfa.next(next);
        if (obj.nfa.match() == match_state::ACCEPT) {
            buf.clear();
        } else if (obj.nfa.match() == match_state::REFUSE) {
            break;
        }
    }
    for (size_t i = buf.size(); i>0; --i) {
        is.putback(buf[i-1]);
    }
    return is;
}

all::all(std::string &dst) : dst(dst) {}

std::istream &operator >>(std::istream &is, all obj) {
    char c = is.get();
    while (is.good()) {
        obj.dst.push_back(c);
        c = is.get();
    }
    return is;
}

template<>
void read_from_string(std::string &&src, std::string &obj) {
    obj = std::move(src);
}

}
