#ifndef NFA_HPP_INCLUDED
#define NFA_HPP_INCLUDED

#include <cstdint>
#include <string>
#include <vector>

// ***************************************************************
// NFA CONSTRUCTION & EXECUTION
// ***************************************************************
namespace nstr_private {

typedef uint8_t symbol_t;

enum class match_state
{
    ACCEPT = 0,
    UNSURE = 1,
    REFUSE = 2
};

enum class trans_t
{
    EXACT,
    NOT_EXACT,
    RANGE,
    NOT_RANGE,
    CLASS,
    NOT_CLASS
};

enum class class_id_t
{
    WORD,
    DIGIT,
    SPACE,
    PUNCT
};

struct trans
{
    trans_t type;

    struct bounds_t
    {
        symbol_t from, to;
    };

    static trans exact(symbol_t sym);
    static trans not_exact(symbol_t sym);
    static trans range(symbol_t from, symbol_t to);
    static trans not_range(symbol_t from, symbol_t to);
    static trans char_class(class_id_t class_id);
    static trans not_char_class(class_id_t class_id);

    union
    {
        bounds_t bounds;
        symbol_t symbol;
        class_id_t class_id;
    };
};

struct nfa_state
{
    std::vector<std::pair<trans, std::vector<int>>> transitions;
    std::vector<int> e_transitions;
    match_state match;

    nfa_state(std::vector<std::pair<trans, std::vector<int>>>&& transitions,
              std::vector<int>&& e_transitions,
              match_state match);
};

class nfa
{
    std::vector<nfa_state> states;
    std::vector<size_t> current;

    static nfa concatenate(nfa&& lhs, nfa&& rhs);
    static nfa loop(nfa&& x);
    static nfa unite(nfa&& lhs, nfa&& rhs);
    static nfa repeat(nfa&& x, int min, int max);
    static nfa parse_regex(const char* regex, size_t size);

    nfa(uint8_t c);
    nfa(const std::vector<trans::bounds_t>& ranges, bool negate);
    nfa();

  public:
    nfa(const std::string& regex);
    nfa(nfa&& other);
    nfa& operator=(nfa&& other);
    nfa(const nfa& other) = default;
    nfa& operator=(const nfa& other) = default;

    const std::vector<nfa_state>& get_states() const;
};

struct nfa_cursor
{
    size_t index;
    size_t count;

    nfa_cursor(size_t index, size_t count);
};

class nfa_executor
{
    nfa state_machine;
    std::vector<nfa_cursor> current;

    void transition_to(const nfa_cursor& cursor,
                       int offset,
                       std::vector<nfa_cursor>& index_set);
    void add_successor_states(uint8_t symbol,
                              const nfa_cursor& cursor,
                              std::vector<nfa_cursor>& index_set);

  public:
    nfa_executor(const std::string& regex);

    void reset();
    void start_path();
    void next(symbol_t symbol);
    match_state match() const;
    size_t longest_match() const;
    size_t trim_short_matches();
};
}
#endif
