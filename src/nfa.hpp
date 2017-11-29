#ifndef NFA_HPP_INCLUDED
#define NFA_HPP_INCLUDED

#include <cstdint>
#include <iostream>
#include <string>
#include <vector>

#include <stdexcept>

// ***************************************************************
// NFA CONSTRUCTION & EXECUTION
// ***************************************************************
namespace nstr_private {

typedef uint32_t symbol_t;

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
    NOT_CLASS,
    ANY
};

enum class class_id_t
{
    WORD,
    DIGIT,
    SPACE,
    PUNCT
};

struct ascii_comparator
{
    static bool is_equal(symbol_t sym1, symbol_t sym2);
    static bool is_equal_to_ascii(symbol_t sym1, char chr);
    static bool is_in_range(symbol_t sym, symbol_t from, symbol_t to);
    static bool is_in_class(symbol_t sym, class_id_t class_id);
    static int to_number(symbol_t sym);
};

struct single_byte_splitter
{
    static symbol_t get(std::istream& str);
    static void putback(std::istream& str, symbol_t sym);
};

struct ascii_backend
    : ascii_comparator
    , single_byte_splitter
{};

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
    static trans any();

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
    static nfa parse_regex(std::istream& regex, size_t level);

    nfa(symbol_t c);
    nfa(const std::vector<trans::bounds_t>& ranges, bool negate);
    nfa();
    nfa(class_id_t class_id, bool negate);
    nfa(trans tr);

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

class nfa_executor_base
{
  protected:
    nfa state_machine;
    std::vector<nfa_cursor> current;

    void transition_to(const nfa_cursor& cursor,
                       int offset,
                       std::vector<nfa_cursor>& index_set);

    nfa_executor_base(const std::string& regex);

  public:
    size_t longest_match() const;
    size_t trim_short_matches();
    match_state match() const;

    void reset();
    void start_path();
};

template<typename BackendT>
class nfa_executor : public nfa_executor_base
{
  private:
    void add_successor_states(symbol_t symbol,
                              const nfa_cursor& cursor,
                              std::vector<nfa_cursor>& index_set);

  public:
    nfa_executor(const std::string& regex);

    void next(symbol_t symbol);
};

template<typename BackendT>
void nfa_executor<BackendT>::add_successor_states(
    symbol_t symbol,
    const nfa_cursor& cursor,
    std::vector<nfa_cursor>& cursor_set)
{
    const auto& state = this->state_machine.get_states()[cursor.index];
    for (const auto& it : state.transitions) {
        bool match;
        void add_successor_states(symbol_t symbol,
                                  const nfa_cursor& cursor,
                                  std::vector<nfa_cursor>& index_set);
        switch (it.first.type) {
            case trans_t::EXACT:
                match = BackendT::is_equal(symbol, it.first.symbol);
                break;
            case trans_t::NOT_EXACT:
                match = !BackendT::is_equal(symbol, it.first.symbol);
                break;
            case trans_t::RANGE:
                match = BackendT::is_in_range(
                    symbol, it.first.bounds.from, it.first.bounds.to);
                break;
            case trans_t::NOT_RANGE:
                match = !BackendT::is_in_range(
                    symbol, it.first.bounds.from, it.first.bounds.to);
                break;
            case trans_t::CLASS:
                match = BackendT::is_in_class(symbol, it.first.class_id);
                break;
            case trans_t::NOT_CLASS:
                match = !BackendT::is_in_class(symbol, it.first.class_id);
                break;
            case trans_t::ANY:
                match = true;
                break;
        }
        if (match) {
            for (size_t i : it.second) {
                this->transition_to(cursor, i, cursor_set);
            }
        }
    }
}

template<typename BackendT>
nfa_executor<BackendT>::nfa_executor(const std::string& regex)
    : nfa_executor_base(regex)
{}

template<typename BackendT>
void nfa_executor<BackendT>::next(symbol_t symbol)
{
    std::vector<nfa_cursor> next;
    next.reserve(this->current.size());
    for (const auto& cursor : this->current) {
        this->add_successor_states(symbol, cursor, next);
    }
    this->current = std::move(next);
}
}
#endif
