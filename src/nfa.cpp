#include "nfa.hpp"
#include "nicestream.hpp"

using namespace nstr;

namespace nstr_private {

nfa_state::nfa_state(std::map<uint8_t, std::vector<int>>&& transitions,
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

nfa::nfa(uint8_t c)
{
    this->states.push_back({ { { c, { 1 } } }, {}, match_state::UNSURE });
    this->states.push_back({ {}, {}, match_state::ACCEPT });
}

nfa::nfa(const std::vector<std::pair<uint8_t, uint8_t>>& ranges, bool negate)
{
    std::map<uint8_t, std::vector<int>> trans;
    if (negate) {
        for (size_t c = 0; c < 256; ++c) {
            bool in_range = false;
            for (const auto& range : ranges) {
                if (c >= range.first && c <= range.second) {
                    in_range = true;
                    break;
                }
            }
            if (!in_range) {
                trans.insert({ c, { 1 } });
            }
        }
    } else {
        for (const auto& range : ranges) {
            for (size_t c = range.first; c <= range.second; ++c) {
                trans.insert({ c, { 1 } });
            }
        }
    }
    this->states.push_back({ std::move(trans), {}, match_state::UNSURE });
    this->states.push_back({ {}, {}, match_state::ACCEPT });
}

nfa::nfa()
{
    this->states.push_back({ {}, {}, match_state::ACCEPT });
}

nfa nfa::parse_regex(const char* regex, size_t size)
{
    std::vector<nfa> sequence;
    bool escape = false;
    for (size_t offset = 0; offset < size; ++offset) {
        const char* base = regex + offset;
        size_t rem = size - offset;
        if (base[0] == '\\' && !escape) {
            escape = true;
            continue;
        } else if (base[0] == '(' && !escape) {
            size_t lhs_begin = 1, lhs_size = 0;
            size_t rhs_begin = 0, rhs_size = 0;
            int level = 1;
            size_t i;
            for (i = 1; i < rem && level > 0; ++i, ++offset) {
                if (base[i] == '\\') {
                    i += 1;
                } else if (base[i] == '(') {
                    ++level;
                } else if (base[i] == ')') {
                    --level;
                } else if (base[i] == '|' && level == 1) {
                    if (rhs_begin != 0) {
                        throw invalid_regex();
                    }
                    lhs_size = i - lhs_begin;
                    rhs_begin = i + 1;
                }
            }
            if (level != 0) {
                throw invalid_regex();
            }
            nfa par;
            if (rhs_begin != 0) {
                rhs_size = i - 1 - rhs_begin;
                nfa lhs = parse_regex(base + lhs_begin, lhs_size);
                nfa rhs = parse_regex(base + rhs_begin, rhs_size);
                par = unite(std::move(lhs), std::move(rhs));
            } else {
                par = parse_regex(base + lhs_begin, i - 1 - lhs_begin);
            }
            sequence.emplace_back(std::move(par));
        } else if (base[0] == '{' && !escape) {
            int min = 0, max = -1;
            bool comma_ok = false, brace_ok = false;
            for (size_t i = 1; i < rem; ++i, ++offset) {
                if (base[i] == ',') {
                    if (i == 1 || comma_ok) {
                        throw invalid_regex();
                    }
                    comma_ok = true;
                } else if (base[i] == '}') {
                    if (i == 1) {
                        throw invalid_regex();
                    }
                    brace_ok = true;
                    ++offset;
                    break;
                } else if (std::isdigit(base[i])) {
                    if (comma_ok) {
                        if (max == -1) {
                            max = 0;
                        }
                        max = max * 10 + base[i] - '0';
                        27f83544e4b6c56d5cd5aa57e169f6d3ead52b3b
                    } else {
                        min = min * 10 + base[i] - '0';
                    }
                } else {
                    throw invalid_regex();
                }
            }
            if (!brace_ok) {
                throw invalid_regex();
            }
            if (!comma_ok) {
                max = min;
            }
            sequence.back() = repeat(std::move(sequence.back()), min, max);
        } else if (base[0] == '*' && !escape) {
            if (offset == 0) {
                throw invalid_regex();
            }
            sequence.back() = loop(std::move(sequence.back()));
        } else if (base[0] == '?' && !escape) {
            if (offset == 0) {
                throw invalid_regex();
            }
            sequence.back() = unite(std::move(sequence.back()), nfa());
        } else if (base[0] == '+' && !escape) {
            if (offset == 0) {
                throw invalid_regex();
            }
            sequence.emplace_back(loop(nfa(sequence.back())));
        } else if (base[0] == '[' && !escape) {
            std::vector<std::pair<uint8_t, uint8_t>> ranges;
            bool brack_esc = false;
            uint8_t prev = 0;
            bool has_prev = false;
            bool is_range = false;
            bool bracket_ok = false;
            bool negate = false;
            for (size_t i = 1; i < rem; ++i, ++offset) {
                if (base[i] == '\\' && !brack_esc) {
                    brack_esc = true;
                    continue;
                } else if (base[i] == '^' && !brack_esc) {
                    if (i != 1) {
                        throw invalid_regex();
                    }
                    negate = true;
                } else if (base[i] == '-' && !brack_esc) {
                    if (!has_prev) {
                        throw invalid_regex();
                    }
                    is_range = true;
                } else if (base[i] == ']' && !brack_esc) {
                    ++offset;
                    bracket_ok = true;
                    break;
                } else {
                    if (is_range) {
                        if (base[i] < prev) {
                            throw invalid_regex();
                        }
                        ranges.emplace_back(prev, base[i]);
                        has_prev = false;
                        is_range = false;
                    } else {
                        if (has_prev) {
                            ranges.emplace_back(prev, prev);
                        }
                        prev = base[i];
                        has_prev = true;
                    }
                }
                brack_esc = false;
            }
            if (is_range || !bracket_ok) {
                throw invalid_regex();
            }
            if (has_prev) {
                ranges.emplace_back(prev, prev);
            }
            sequence.emplace_back(nfa(ranges, negate));
        } else if (base[0] == '.' && !escape) {
            sequence.emplace_back(nfa({ { 0, 255 } }, false));
        } else if (escape) {
            std::vector<std::pair<uint8_t, uint8_t>> ranges;
            bool negate = false;
            switch (base[0]) {
                case 'd':
                    ranges = { { '0', '9' } };
                    break;
                case 'D':
                    ranges = { { '0', '9' } };
                    negate = true;
                    break;
                case 'w':
                    ranges = {
                        { 'a', 'z' }, { 'A', 'Z' }, { '_', '_' }, { '0', '9' }
                    };
                    break;
                case 'W':
                    ranges = {
                        { 'a', 'z' }, { 'A', 'Z' }, { '_', '_' }, { '0', '9' }
                    };
                    negate = true;
                    break;
                case 's':
                    ranges = { { ' ', ' ' }, { '\t', '\r' } };
                    break;
                case 'S':
                    ranges = { { ' ', ' ' }, { '\t', '\r' } };
                    negate = true;
                    break;
                default:
                    ranges = { { base[0], base[0] } };
            }
            sequence.emplace_back(nfa(ranges, negate));
        } else {
            sequence.emplace_back(nfa(base[0]));
        }
        escape = false;
    }
    if (sequence.empty()) {
        return nfa();
    } else {
        for (size_t i = 1; i < sequence.size(); ++i) {
            sequence.front() = concatenate(std::move(sequence.front()),
                                           std::move(sequence[i]));
        }
        return std::move(sequence.front());
    }
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

nfa::nfa(const std::string& regex)
{
    *this = parse_regex(regex.c_str(), regex.size());
}

nfa_cursor::nfa_cursor(size_t index, size_t count)
    : index(index)
    , count(count)
{}

void nfa_executor::transition_to(const nfa_cursor& cursor,
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

void nfa_executor::add_successor_states(uint8_t symbol,
                                        const nfa_cursor& cursor,
                                        std::vector<nfa_cursor>& cursor_set)
{
    const auto& state = this->state_machine.get_states()[cursor.index];
    const auto it = state.transitions.find(symbol);
    if (it == state.transitions.end()) {
        return;
    }
    for (size_t i : it->second) {
        this->transition_to(cursor, i, cursor_set);
    }
}

void nfa_executor::start_path()
{
    this->transition_to({ 0, 0 }, 0, this->current);
}

void nfa_executor::next(uint8_t symbol)
{
    std::vector<nfa_cursor> next;
    next.reserve(this->current.size());
    for (const auto& cursor : this->current) {
        this->add_successor_states(symbol, cursor, next);
    }
    this->current = std::move(next);
}

match_state nfa_executor::match() const
{
    match_state result = match_state::REFUSE;
    for (size_t i = this->current.size(); i > 0; --i) {
        const match_state match_i =
            this->state_machine.get_states()[this->current[i - 1].index].match;
        result = std::min(result, match_i);
    }
    return result;
}

void nfa_executor::reset()
{
    this->current.clear();
    this->start_path();
}

size_t nfa_executor::longest_match() const
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

size_t nfa_executor::trim_short_matches()
{
    size_t max = this->longest_match();
    for (size_t i = this->current.size(); i > 0; --i) {
        if (this->current[i - 1].count != max) {
            this->current.erase(this->current.begin() + i);
        }
    }
    return max;
}

nfa_executor::nfa_executor(const std::string& regex)
    : state_machine(regex)
{
    this->start_path();
}
}
