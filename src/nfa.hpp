#ifndef NFA_HPP_INCLUDED
#define NFA_HPP_INCLUDED

#include <map>
#include <vector>
#include <cstdint>

// ***************************************************************
// NFA CONSTRUCTION & EXECUTION
// ***************************************************************
namespace nstr_private {

enum class match_state {
    ACCEPT = 0, UNSURE = 1, REFUSE = 2
};

struct nfa_state {
    std::map<uint8_t, std::vector<int>> transitions;
    std::vector<int> e_transitions;
    match_state match;

    nfa_state(std::map<uint8_t, std::vector<int>> &&transitions, std::vector<int> &&e_transitions, match_state match);
};

class nfa {
    std::vector<nfa_state> states;
    std::vector<size_t> current;

    static nfa concatenate(nfa&& lhs, nfa&& rhs);
    static nfa loop(nfa&& x);
    static nfa unite(nfa&& lhs, nfa&& rhs);
    static nfa repeat(nfa&& x, int min, int max);
    static nfa parse_regex(const char *regex, size_t size);

    nfa(uint8_t c);
    nfa(const std::vector<std::pair<uint8_t, uint8_t>> &ranges, bool negate);
    nfa();

public:
    nfa(const std::string& regex);
    nfa(nfa&& other);
    nfa& operator=(nfa&& other);
    nfa(const nfa& other) = default;
    nfa& operator=(const nfa& other) = default;

    const std::vector<nfa_state> &get_states() const;
};

struct nfa_cursor {
    size_t index;
    size_t count;

    nfa_cursor(size_t index, size_t count);
};

class nfa_executor {
    nfa state_machine;
    std::vector<nfa_cursor> current;

    void transition_to(const nfa_cursor &cursor, int offset, std::vector<nfa_cursor>& index_set);
    void add_successor_states(uint8_t symbol, const nfa_cursor& cursor, std::vector<nfa_cursor>& index_set);

public:
    nfa_executor(const std::string &regex);

    void next(uint8_t symbol);
    match_state match() const;
    size_t longest_match() const;
};
}
#endif
