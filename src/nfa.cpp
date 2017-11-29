#include "nfa.hpp"
#include "nicestream.hpp"

using namespace nstr;

namespace nstr_private {

nfa_state::nfa_state(
    std::vector<std::pair<trans, std::vector<int>>>&& transitions,
    std::vector<int>&& e_transitions,
    match_state match)
    : transitions(std::move(transitions))
    , e_transitions(std::move(e_transitions))
    , match(match)
{}

nfa nfa::concatenate(nfa&& lhs, nfa&& rhs)
{
    for (size_t i = 0; i < lhs.states.size(); ++i) {
        if (lhs.states[i].match == match_state::ACCEPT) {
            lhs.states[i].e_transitions.push_back(lhs.states.size() - i);
            lhs.states[i].match = match_state::UNSURE;
        }
    }
    lhs.states.insert(lhs.states.end(), rhs.states.begin(), rhs.states.end());
    return std::move(lhs);
}

nfa nfa::loop(nfa&& x)
{
    x.states.emplace_back(nfa_state({}, {}, match_state::ACCEPT));
    x.states.insert(
        x.states.begin(),
        { {}, { 1, static_cast<int>(x.states.size()) }, match_state::UNSURE });
    for (size_t i = 1; i < x.states.size() - 1; ++i) {
        if (x.states[i].match == match_state::ACCEPT) {
            x.states[i].e_transitions.push_back(x.states.size() - 1 - i);
            x.states[i].e_transitions.push_back(-static_cast<int>(i) + 1);
            x.states[i].match = match_state::UNSURE;
        }
    }
    return std::move(x);
}

nfa nfa::unite(nfa&& lhs, nfa&& rhs)
{
    lhs.states.insert(lhs.states.begin(),
                      { {},
                        { 1, static_cast<int>(lhs.states.size() + 1) },
                        match_state::UNSURE });
    lhs.states.insert(lhs.states.end(), rhs.states.begin(), rhs.states.end());
    return std::move(lhs);
}

nfa nfa::repeat(nfa&& x, int min, int max)
{
    nfa result;
    nfa x_orig = std::move(x);
    for (int i = 0; i < min; ++i) {
        result = concatenate(std::move(result), nfa(x_orig));
    }
    if (max == -1) {
        result = concatenate(std::move(result), loop(std::move(x_orig)));
    } else {
        if (max < min) {
            throw invalid_regex();
        }
        for (int i = 0; i < max - min; ++i) {
            for (size_t j = 0; j < result.states.size(); ++j) {
                if (result.states[j].match == match_state::ACCEPT) {
                    result.states[j].e_transitions.push_back(
                        result.states.size() - j);
                }
            }
            result.states.insert(result.states.end(),
                                 x_orig.states.begin(),
                                 x_orig.states.end());
        }
    }
    return result;
}

nfa::nfa(symbol_t c)
{
    this->states.push_back(
        { { { trans::exact(c), { 1 } } }, {}, match_state::UNSURE });
    this->states.push_back({ {}, {}, match_state::ACCEPT });
}

nfa::nfa(const std::vector<trans::bounds_t>& ranges, bool negate)
{
    std::vector<std::pair<trans, std::vector<int>>> transitions;
    std::vector<int> succ0{ 1 };
    for (const auto& range : ranges) {
        if (negate) {
            transitions.emplace_back(trans::not_range(range.from, range.to),
                                     succ0);
        } else {
            transitions.emplace_back(trans::range(range.from, range.to), succ0);
        }
    }
    this->states.push_back({ std::move(transitions), {}, match_state::UNSURE });
    this->states.push_back({ {}, {}, match_state::ACCEPT });
}

nfa::nfa()
{
    this->states.push_back({ {}, {}, match_state::ACCEPT });
}

nfa::nfa(class_id_t class_id, bool negate)
{
    std::vector<std::pair<trans, std::vector<int>>> transitions;
    std::vector<int> succ0{ 1 };
    if (negate) {
        transitions.emplace_back(trans::not_char_class(class_id), succ0);
    } else {
        transitions.emplace_back(trans::char_class(class_id), succ0);
    }
    this->states.push_back({ std::move(transitions), {}, match_state::UNSURE });
    this->states.push_back({ {}, {}, match_state::ACCEPT });
}

nfa::nfa(trans tr)
{
    std::vector<std::pair<trans, std::vector<int>>> transitions;
    std::vector<int> succ0{ 1 };
    transitions.emplace_back(tr, succ0);
    this->states.push_back({ std::move(transitions), {}, match_state::UNSURE });
    this->states.push_back({ {}, {}, match_state::ACCEPT });
}

const std::vector<nfa_state>& nfa::get_states() const
{
    return this->states;
}

nfa& nfa::operator=(nfa&& other)
{
    if (this != &other) {
        this->states = std::move(other.states);
        this->current = std::move(other.current);
    }
    return *this;
}

nfa::nfa(nfa&& other)
{
    *this = std::move(other);
}

nfa_cursor::nfa_cursor(size_t index, size_t count)
    : index(index)
    , count(count)
{}

void nfa_executor_base::transition_to(const nfa_cursor& cursor,
                                      int offset,
                                      std::vector<nfa_cursor>& index_set)
{
    const auto& state = this->state_machine.get_states()[cursor.index + offset];
    for (size_t i : state.e_transitions) {
        this->transition_to(cursor, offset + i, index_set);
    }
    if (state.match != match_state::REFUSE) {
        index_set.emplace_back(cursor.index + offset, cursor.count + 1);
    }
}

void nfa_executor_base::start_path()
{
    this->transition_to({ 0, 0 }, 0, this->current);
}

match_state nfa_executor_base::match() const
{
    match_state result = match_state::REFUSE;
    for (size_t i = this->current.size(); i > 0; --i) {
        const match_state match_i =
            this->state_machine.get_states()[this->current[i - 1].index].match;
        result = std::min(result, match_i);
    }
    return result;
}

void nfa_executor_base::reset()
{
    this->current.clear();
    this->start_path();
}

size_t nfa_executor_base::longest_match() const
{
    size_t result = 1;
    for (size_t i = this->current.size(); i > 0; --i) {
        const match_state match_i =
            this->state_machine.get_states()[this->current[i - 1].index].match;
        if (match_i == match_state::ACCEPT) {
            result = std::max(current[i - 1].count, result);
        }
    }
    return result - 1;
}

size_t nfa_executor_base::trim_short_matches()
{
    size_t max = this->longest_match();
    for (size_t i = this->current.size(); i > 0; --i) {
        if (this->current[i - 1].count != max) {
            this->current.erase(this->current.begin() + i);
        }
    }
    return max;
}

trans trans::exact(symbol_t sym)
{
    trans res;
    res.type = trans_t::EXACT;
    res.symbol = sym;
    return res;
}

trans trans::not_exact(symbol_t sym)
{
    trans res;
    res.type = trans_t::NOT_EXACT;
    res.symbol = sym;
    return res;
}

trans trans::range(symbol_t from, symbol_t to)
{
    trans res;
    res.type = trans_t::RANGE;
    res.bounds.from = from;
    res.bounds.to = to;
    return res;
}

trans trans::not_range(symbol_t from, symbol_t to)
{
    trans res;
    res.type = trans_t::NOT_RANGE;
    res.bounds.from = from;
    res.bounds.to = to;
    return res;
}

trans trans::char_class(class_id_t class_id)
{
    trans res;
    res.type = trans_t::CLASS;
    res.class_id = class_id;
    return res;
}

trans trans::not_char_class(class_id_t class_id)
{
    trans res;
    res.type = trans_t::NOT_CLASS;
    res.class_id = class_id;
    return res;
}

trans trans::any()
{
    trans res;
    res.type = trans_t::ANY;
    return res;
}

bool ascii_comparator::is_equal(symbol_t sym1, symbol_t sym2)
{
    return sym1 == sym2;
}

bool ascii_comparator::is_equal_to_ascii(symbol_t sym1, char chr)
{
    return sym1 == chr;
}

bool ascii_comparator::is_in_range(symbol_t sym, symbol_t from, symbol_t to)
{
    return sym >= from && sym <= to;
}

bool ascii_comparator::is_in_class(symbol_t sym, class_id_t class_id)
{
    switch (class_id) {
        case class_id_t::DIGIT:
            return std::isdigit(sym);
        case class_id_t::PUNCT:
            return std::ispunct(sym);
        case class_id_t::SPACE:
            return std::isspace(sym);
        case class_id_t::WORD:
            return std::isalnum(sym) || sym == '_';
    }
}

int ascii_comparator::to_number(symbol_t sym)
{
    return sym - '0';
}

symbol_t single_byte_splitter::get(std::istream& str)
{
    return str.get();
}

void single_byte_splitter::putback(std::istream& str, symbol_t sym)
{
    str.putback(sym);
}

nstr_private::nfa_executor_base::nfa_executor_base(nfa&& state_machine)
    : state_machine(state_machine)
{
    this->start_path();
}
}
