#include "nicestream.hpp"
#include <cstdint>
#include <string>
#include <map>
#include <vector>

namespace nice {
// ***************************************************************
// NFA CONSTRUCTION & EXECUTION
// ***************************************************************
enum class match_state {
    ACCEPT = 0, UNSURE = 1, REFUSE = 2
};

struct nfa_state {
    std::map<uint8_t, std::vector<int>> transitions;
    std::vector<int> e_transitions;
    match_state match;

    nfa_state(std::map<uint8_t, std::vector<int>> &&transitions, std::vector<int> &&e_transitions, match_state match);
};
nfa_state::nfa_state(std::map<uint8_t, std::vector<int>> &&transitions, std::vector<int> &&e_transitions, match_state match) :
    transitions(std::move(transitions)), e_transitions(std::move(e_transitions)), match(match) {}

class nfa {
    std::vector<nfa_state> states;
    std::vector<size_t> current;

    void transition_to(size_t index, std::vector<size_t>& index_set);
    void add_successor_states(uint8_t symbol, size_t index, std::vector<size_t>& index_set);

    static nfa concatenate(nfa&& lhs, nfa&& rhs);
    static nfa repeat(nfa&& x);
    static nfa unite(nfa&& lhs, nfa&& rhs);
    static nfa parse_regex(const char *regex, size_t size);

    nfa(uint8_t c);
    nfa();

public:
    nfa(const std::string& regex);
    nfa(nfa&& other);
    nfa& operator=(nfa&& other);
    nfa(const nfa& other) = default;
    nfa& operator=(const nfa& other) = default;

    
    void next(uint8_t symbol);
    match_state match() const;
};

void nfa::transition_to(size_t index, std::vector<size_t>& index_set) {
    const auto& state = this->states[index];
    for (size_t i : state.e_transitions) {
        this->transition_to(index + i, index_set);
    }
    if (state.match != match_state::REFUSE) {
        index_set.push_back(index);
    }
}

void nfa::add_successor_states(uint8_t symbol, size_t index, std::vector<size_t>& index_set) {
    const auto& state = this->states[index];
    const auto it = state.transitions.find(symbol);
    if (it == state.transitions.end()) {
        return;
    }
    for (size_t i : it->second) {
        this->transition_to(index + i, index_set);
    }
}

void nfa::next(uint8_t symbol) {
    std::vector<size_t> next;
    next.reserve(this->current.size());
    for (const auto& index : this->current) {
        this->add_successor_states(symbol, index, next);
    }
    this->current = std::move(next);
}

match_state nfa::match() const {
    match_state result = match_state::REFUSE;
    for (size_t i = this->current.size(); i>0; --i) {
        const match_state match_i = this->states[this->current[i-1]].match;
        result = std::min(result, match_i);
    }
    return result;
}

nfa nfa::concatenate(nfa &&lhs, nfa &&rhs) {
    for (size_t i = 0; i<lhs.states.size(); ++i) {
        if (lhs.states[i].match == match_state::ACCEPT) {
            lhs.states[i].e_transitions.push_back(lhs.states.size() - i);
            lhs.states[i].match = match_state::UNSURE;
        }
    }
    lhs.states.insert(lhs.states.end(), rhs.states.begin(), rhs.states.end());
    return std::move(lhs);
}

nfa nfa::repeat(nfa &&x) {
    x.states.emplace_back(nfa_state({}, {}, match_state::ACCEPT));
    x.states.insert(x.states.begin(), {{}, {1, static_cast<int>(x.states.size())}, match_state::UNSURE});
    for (size_t i = 1; i<x.states.size()-1; ++i) {
        if (x.states[i].match == match_state::ACCEPT) {
            x.states[i].e_transitions.push_back(x.states.size()-1-i);
            x.states[i].e_transitions.push_back(-static_cast<int>(i)+1);
            x.states[i].match = match_state::UNSURE;
        }
    }
    return std::move(x);
}

nfa nfa::unite(nfa &&lhs, nfa &&rhs) {
    lhs.states.insert(lhs.states.begin(), {{}, {1, static_cast<int>(lhs.states.size()+1)}, match_state::UNSURE});
    lhs.states.insert(lhs.states.end(), rhs.states.begin(), rhs.states.end());
    return std::move(lhs);
}

nfa::nfa(uint8_t c) {
    this->states.push_back({{{c, {1}}}, {}, match_state::UNSURE});
    this->states.push_back({{}, {}, match_state::ACCEPT});
}

nfa::nfa() {
    this->states.push_back({{}, {}, match_state::ACCEPT});
}

nfa nfa::parse_regex(const char *regex, size_t size) {
    std::vector<nfa> sequence;
    bool escape = false;
    for (size_t offset = 0; offset<size; ++offset) {
        const char *base = regex + offset;
        size_t rem = size - offset;
        if (base[0] == '\\' && !escape) {
            escape = true;
            continue;
        } else if (base[0] == '(' && !escape) {
            size_t lhs_begin = 1, lhs_size = 0;
            size_t rhs_begin = 0, rhs_size = 0;
            int level = 1;
            size_t i;
            for (i = 1; i<rem && level > 0; ++i) {
                if (base[i] == '\\') {
                    i += 1;
                } else if (base[i] == '(') {
                    ++level;
                } else if (base[i] == ')') {
                    --level;
                } else if (base[i] == '|' && level == 1) {
                    lhs_size = i - lhs_begin;
                    rhs_begin = i+1;
                }
            }
            if (level != 0) {
                throw invalid_regex();
            }
            nfa par;
            if (rhs_begin != 0) {
                rhs_size = i-1-rhs_begin;
                nfa lhs = parse_regex(base + lhs_begin, lhs_size);
                nfa rhs = parse_regex(base + rhs_begin, rhs_size);
                par = unite(std::move(lhs), std::move(rhs));
            } else {
                par = parse_regex(base + lhs_begin, i-1-lhs_begin);
            }
            sequence.emplace_back(std::move(par));
            offset += i-1;
        } else if (base[0] == '*' && !escape) {
            if (offset == 0) {
                throw invalid_regex();
            }
            sequence.back() = repeat(std::move(sequence.back()));
        } else {
            sequence.emplace_back(nfa(base[0]));
        }
        escape = false;

    }
    if (sequence.empty()) {
        return nfa();
    } else {
        for (size_t i = 1; i<sequence.size(); ++i) {
            sequence.front() = concatenate(std::move(sequence.front()), std::move(sequence[i]));
        }
        return std::move(sequence.front());
    }
}

nfa& nfa::operator=(nfa &&other) {
    if (this != &other) {
        this->states = std::move(other.states);
        this->current = std::move(other.current);
    }
    return *this;
}

nfa::nfa(nfa &&other) {
    *this = std::move(other);
}

nfa::nfa(const std::string& regex) {
    *this = parse_regex(regex.c_str(), regex.size());
    this->transition_to(0, this->current);
}

// *************************************************************
// STREAM STUFF
// *************************************************************

sep::sep(const std::string& regex) : nfa_p(new nfa(regex))
{
}

sep::~sep() {
    delete nfa_p;
}

sep::sep(const sep& other) : nfa_p(nullptr) {
    *this = other;
}

sep &sep::operator=(const sep& other) {
    if (this != &other) {
        delete this->nfa_p;
        this->nfa_p = new nfa(*other.nfa_p);
    }
    return *this;
}

sep::sep(const sep&& other) : nfa_p(nullptr) {
    *this = std::move(other);
}

sep &sep::operator=(sep&& other) {
    if (this != &other) {
        std::swap(this->nfa_p, other.nfa_p);
    }
    return *this;
}

std::istream &operator >>(std::istream &is, sep what) {
    bool is_valid = false;
    while (true) {
        is_valid = is_valid || what.nfa_p->match() == match_state::ACCEPT;
        uint8_t next = static_cast<uint8_t>(is.peek());
        what.nfa_p->next(next);
        if (what.nfa_p->match() == match_state::REFUSE) {
            break;
        }
        is.get();
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
