#include "nicestream.hpp"
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

sep::sep(const std::string& regex) : executor(regex)
{
}

std::istream &operator >>(std::istream &is, sep what) {
    bool is_valid = what.executor.match() == match_state::ACCEPT;
    std::vector<uint8_t> buf;
    while (true) {
        uint8_t next = static_cast<uint8_t>(is.get());
        buf.push_back(next);
        what.executor.next(next);
        if (what.executor.match() == match_state::ACCEPT) {
            is_valid = true;
            buf.clear();
        } else if (what.executor.match() == match_state::REFUSE) {
            break;
        }
    }
    for (size_t i = buf.size(); i>0; --i) {
        is.putback(buf[i-1]);
    }
    if (!is_valid) {
        throw invalid_input();
    }
    return is;
}

std::istream& operator >>(std::istream& is, skip<>) {
    return is;
}
}
