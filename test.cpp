#define CATCH_CONFIG_MAIN
#include <catch.hpp>
#include <string>
#include <sstream>

#include "src/nicestream.hpp"
#include "src/nfa.hpp"

using namespace nstr;
using namespace nstr_private;
typedef std::stringstream sstr;

TEST_CASE("Regex parsing", "[regex]") {
    // simple literals
    CHECK_NOTHROW(sep("abcdz!!--<>\n\t^^^"));
    CHECK_NOTHROW(sep("abc\\.\\*\\?\\+\\[\\"));
    CHECK_NOTHROW(sep("(abcabc)(cabcab)"));
    CHECK_NOTHROW(sep("(ab(cabc)(ca(bc))ab)"));
    CHECK_THROWS_AS(sep("(zzz"), invalid_regex);
    CHECK_THROWS_AS(sep("(zz(z)"), invalid_regex);

    // ?, *, +
    CHECK_NOTHROW(sep("x?"));
    CHECK_NOTHROW(sep("(xy)?"));
    CHECK_NOTHROW(sep("x.?"));
    CHECK_THROWS_AS(sep("?zzz"), invalid_regex);
    CHECK_NOTHROW(sep("x*"));
    CHECK_NOTHROW(sep("(xy)*"));
    CHECK_NOTHROW(sep("x.*"));
    CHECK_THROWS_AS(sep("*zzz"), invalid_regex);
    CHECK_NOTHROW(sep("x+"));
    CHECK_NOTHROW(sep("(xy)+"));
    CHECK_NOTHROW(sep("x.+"));
    CHECK_THROWS_AS(sep("+zzz"), invalid_regex);

    // quantifiers
    CHECK_NOTHROW(sep("x{4}"));
    CHECK_NOTHROW(sep("(xy){4}"));
    CHECK_NOTHROW(sep("x{4,}"));
    CHECK_NOTHROW(sep("(xy){4,}"));
    CHECK_NOTHROW(sep("x{4,30}"));
    CHECK_NOTHROW(sep("(xy){4,30}"));
    CHECK_THROWS_AS(sep("x{,5}"), invalid_regex);
    CHECK_THROWS_AS(sep("x{,}"), invalid_regex);
    CHECK_THROWS_AS(sep("x{}"), invalid_regex);
    CHECK_THROWS_AS(sep("x{10,7}"), invalid_regex);
    CHECK_THROWS_AS(sep("x{blah}"), invalid_regex);
    CHECK_THROWS_AS(sep("x{1,7,8,9}"), invalid_regex);
    CHECK_THROWS_AS(sep("x{1,7"), invalid_regex);
    CHECK_THROWS_AS(sep("x{1,"), invalid_regex);
    CHECK_THROWS_AS(sep("x{1"), invalid_regex);

    // character classes
    CHECK_NOTHROW(sep("[a-k7-9%=]"));
    CHECK_NOTHROW(sep("[a\\-*\\][]"));
    CHECK_NOTHROW(sep("\\d\\D\\w\\W\\s\\S"));
    CHECK_NOTHROW(sep("[^a-z678]"));
    CHECK_NOTHROW(sep("[]")); // FIXME: should this be accepted?
    CHECK_NOTHROW(sep("[^]")); // FIXME: should this be accepted?
    CHECK_THROWS_AS(sep("[a^b]"), invalid_regex);
    CHECK_THROWS_AS(sep("[a-b"), invalid_regex);
    CHECK_THROWS_AS(sep("[a-b-c]"), invalid_regex);
    CHECK_THROWS_AS(sep("[z-b]"), invalid_regex);
    CHECK_THROWS_AS(sep("[-z]"), invalid_regex);
    CHECK_THROWS_AS(sep("[z-]"), invalid_regex);

    // unions
    CHECK_NOTHROW(sep("(aaaa|bbb)"));
    CHECK_NOTHROW(sep("((a(aa|)a|bbb)|zzzz)"));
    CHECK_NOTHROW(sep("(a|)"));
    CHECK_NOTHROW(sep("(a|\\|a)"));
    CHECK_THROWS_AS(sep("(a|b|c)"), invalid_regex); // FIXME: maybe this isn't invalid
    CHECK_THROWS_AS(sep("(a|c()"), invalid_regex);
}

TEST_CASE("nstr::skip", "[skip]") {
    {
        int i, j;
        sstr("10 20 30 40") >> i >> skip<int, int>() >> j;
        CHECK(i == 10);
        CHECK(j == 40);
    }
    {
        int i, j;
        sstr("10 20 30 40") >> i >> skip<int>() >> j;
        CHECK(i == 10);
        CHECK(j == 30);
    }
    {
        int i, j;
        sstr("10 20 30 40") >> i >> skip<>() >> j;
        CHECK(i == 10);
        CHECK(j == 20);
    }
    {
        int i;
        std::string s;
        sstr("10 20 abc def") >> i >> skip<int, std::string>() >> s;
        CHECK(i == 10);
        CHECK(s == "def");
    }
}

TEST_CASE("Match lengths", "[length]") {
    {
        nfa_executor e("a*");
        CHECK(e.match() == match_state::ACCEPT);
        CHECK(e.longest_match() == 0);
        e.next('a');
        CHECK(e.match() == match_state::ACCEPT);
        CHECK(e.longest_match() == 1);
        e.next('a');
        CHECK(e.match() == match_state::ACCEPT);
        CHECK(e.longest_match() == 2);
        e.next('a');
        CHECK(e.match() == match_state::ACCEPT);
        CHECK(e.longest_match() == 3);
    }
    {
        nfa_executor e("(a{3}|a)");
        CHECK(e.match() == match_state::UNSURE);
        CHECK(e.longest_match() == 0);
        e.next('a');
        CHECK(e.match() == match_state::ACCEPT);
        CHECK(e.longest_match() == 1);
        e.next('a');
        CHECK(e.match() == match_state::UNSURE);
        CHECK(e.longest_match() == 0);
        e.next('a');
        CHECK(e.match() == match_state::ACCEPT);
        CHECK(e.longest_match() == 3);
        e.next('a');
        CHECK(e.match() == match_state::REFUSE);
        CHECK(e.longest_match() == 0);
    }
}
