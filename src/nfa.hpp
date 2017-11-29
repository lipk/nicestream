#ifndef NFA_HPP_INCLUDED
#define NFA_HPP_INCLUDED

#include <cstdint>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include <stdexcept>

namespace nstr {
struct invalid_input : public std::exception
{};
struct stream_error : public std::exception
{};
struct invalid_regex : public std::exception
{};
}

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
    template<typename BackendT>
    friend nfa parse_regex(std::istream& regex, size_t level);

    std::vector<nfa_state> states;
    std::vector<size_t> current;

    static nfa concatenate(nfa&& lhs, nfa&& rhs);
    static nfa loop(nfa&& x);
    static nfa unite(nfa&& lhs, nfa&& rhs);
    static nfa repeat(nfa&& x, int min, int max);

    nfa(symbol_t c);
    nfa(const std::vector<trans::bounds_t>& ranges, bool negate);
    nfa();
    nfa(class_id_t class_id, bool negate);
    nfa(trans tr);

  public:
    nfa(nfa&& other);
    nfa& operator=(nfa&& other);
    nfa(const nfa& other) = default;
    nfa& operator=(const nfa& other) = default;

    const std::vector<nfa_state>& get_states() const;
};

template<typename BackendT>
nfa parse_regex(std::istream& str, size_t level)
{
    if (level >= 100) {
        throw nstr::invalid_regex();
    }
    std::vector<nfa> sequence;
    bool has_parallel = false;
    nfa parallel;
    bool escape = false;
    for (symbol_t cur = BackendT::get(str); !str.eof();
         cur = BackendT::get(str)) {
        if (BackendT::is_equal_to_ascii(cur, '\\') && !escape) {
            escape = true;
            continue;
        } else if (BackendT::is_equal_to_ascii(cur, '(') && !escape) {
            nfa sub = parse_regex<BackendT>(str, level + 1);
            sequence.emplace_back(std::move(sub));
        } else if (BackendT::is_equal_to_ascii(cur, '{') && !escape) {
            int min = 0, max = -1;
            bool comma_ok = false, brace_ok = false, first_it = true;
            for (cur = BackendT::get(str); !str.eof();
                 cur = BackendT::get(str)) {
                if (BackendT::is_equal_to_ascii(cur, ',')) {
                    if (comma_ok || first_it) {
                        throw nstr::invalid_regex();
                    }
                    comma_ok = true;
                } else if (BackendT::is_equal_to_ascii(cur, '}')) {
                    if (first_it) {
                        throw nstr::invalid_regex();
                    }
                    brace_ok = true;
                    break;
                } else if (BackendT::is_in_class(cur, class_id_t::DIGIT)) {
                    int value = BackendT::to_number(cur);
                    if (comma_ok) {
                        if (max == -1) {
                            max = 0;
                        }
                        max = max * 10 + value;
                    } else {
                        min = min * 10 + value;
                    }
                } else {
                    throw nstr::invalid_regex();
                }
                first_it = false;
            }
            if (!brace_ok) {
                throw nstr::invalid_regex();
            }
            if (!comma_ok) {
                max = min;
            }
            if (sequence.empty()) {
                throw nstr::invalid_regex();
            }
            sequence.back() = nfa::repeat(std::move(sequence.back()), min, max);
        } else if (BackendT::is_equal_to_ascii(cur, '*') && !escape) {
            if (sequence.empty()) {
                throw nstr::invalid_regex();
            }
            sequence.back() = nfa::loop(std::move(sequence.back()));
        } else if (BackendT::is_equal_to_ascii(cur, '?') && !escape) {
            if (sequence.empty()) {
                throw nstr::invalid_regex();
            }
            sequence.back() = nfa::unite(std::move(sequence.back()), nfa());
        } else if (BackendT::is_equal_to_ascii(cur, '+') && !escape) {
            if (sequence.empty()) {
                throw nstr::invalid_regex();
            }
            sequence.emplace_back(nfa::loop(nfa(sequence.back())));
        } else if (BackendT::is_equal_to_ascii(cur, '[') && !escape) {
            std::vector<trans::bounds_t> ranges;
            bool brack_esc = false;
            symbol_t prev = 0;
            bool has_prev = false, is_range = false, bracket_ok = false,
                 negate = false;
            bool first_iter = true;
            for (cur = BackendT::get(str); !str.eof();
                 cur = BackendT::get(str), first_iter = false) {
                if (BackendT::is_equal_to_ascii(cur, '\\') && !brack_esc) {
                    brack_esc = true;
                    continue;
                } else if (BackendT::is_equal_to_ascii(cur, '^') &&
                           !brack_esc) {
                    if (!first_iter) {
                        throw nstr::invalid_regex();
                    }
                    negate = true;
                } else if (BackendT::is_equal_to_ascii(cur, '-') &&
                           !brack_esc) {
                    if (!has_prev) {
                        throw nstr::invalid_regex();
                    }
                    is_range = true;
                } else if (BackendT::is_equal_to_ascii(cur, ']') &&
                           !brack_esc) {
                    bracket_ok = true;
                    break;
                } else {
                    if (is_range) {
                        ranges.push_back({ prev, cur });
                        has_prev = false;
                        is_range = false;
                    } else {
                        if (has_prev) {
                            ranges.push_back({ prev, prev });
                        }
                        prev = cur;
                        has_prev = true;
                    }
                }
                brack_esc = false;
            }
            if (is_range || !bracket_ok) {
                throw nstr::invalid_regex();
            }
            if (has_prev) {
                ranges.push_back({ prev, prev });
            }
            sequence.emplace_back(nfa(ranges, negate));
        } else if (BackendT::is_equal_to_ascii(cur, '.') && !escape) {
            sequence.emplace_back(nfa(trans::any()));
        } else if (BackendT::is_equal_to_ascii(cur, ')') && !escape &&
                   level > 0) {
            break;
        } else if (BackendT::is_equal_to_ascii(cur, '|') && !escape &&
                   level > 0) {
            parallel = parse_regex<BackendT>(str, level);
            has_parallel = true;
            break;
        } else if (escape) {
            class_id_t id;
            bool negate = false;
            bool is_char_class = true;
            switch (cur) {
                case 'D':
                    negate = true;
                case 'd':
                    id = class_id_t::DIGIT;
                    break;
                case 'W':
                    negate = true;
                case 'w':
                    id = class_id_t::WORD;
                    break;
                case 'S':
                    negate = true;
                case 's':
                    id = class_id_t::SPACE;
                    break;
                default:
                    is_char_class = false;
            }
            if (is_char_class) {
                sequence.emplace_back(nfa(id, negate));
            } else {
                sequence.emplace_back(nfa(cur));
            }
        } else {
            sequence.emplace_back(nfa(cur));
        }
        escape = false;
    }
    if (level > 0 && str.eof()) {
        throw nstr::invalid_regex();
    }
    nfa res;
    if (sequence.empty()) {
        res = nfa();
    } else {
        for (size_t i = 1; i < sequence.size(); ++i) {
            sequence.front() = nfa::concatenate(std::move(sequence.front()),
                                                std::move(sequence[i]));
        }
        res = std::move(sequence.front());
    }
    if (has_parallel) {
        res = nfa::unite(std::move(res), std::move(parallel));
    }
    return res;
}

template<typename BackendT>
nfa parse_regex(const std::string& regex)
{
    std::stringstream str(regex);
    return parse_regex<BackendT>(str, 0);
}

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

    nfa_executor_base(nfa&& state_machine);

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
    : nfa_executor_base(parse_regex<BackendT>(regex))
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
