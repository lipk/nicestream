#include <catch.hpp>
#include <string>
#include <sstream>
#include <list>
#include <set>

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

TEST_CASE("nstr::sep", "[sep]") {
    {
        int i, j;
        sstr ss("10,20");
        ss >> i >> sep(",") >> j;
        CHECK(i == 10);
        CHECK(j == 20);
    }
    {
        int i, j;
        sstr ss("10,,,,,,20");
        ss >> i >> sep(",+") >> j;
        CHECK(i == 10);
        CHECK(j == 20);
    }
    {
        int i, j;
        sstr ss("10;20");
        CHECK_THROWS_AS(ss >> i >> sep(",") >> j, invalid_input);
    }
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
    {
        nfa_executor e("(b|aba)");
        CHECK(e.match() == match_state::UNSURE);
        CHECK(e.longest_match() == 0);
        e.next('a');
        e.start_path();
        CHECK(e.match() == match_state::UNSURE);
        CHECK(e.longest_match() == 0);
        e.next('b');
        e.start_path();
        CHECK(e.match() == match_state::ACCEPT);
        CHECK(e.longest_match() == 1);
        e.next('a');
        e.start_path();
        CHECK(e.match() == match_state::ACCEPT);
        CHECK(e.longest_match() == 3);
        e.next('a');
        e.start_path();
        CHECK(e.match() == match_state::UNSURE);
        CHECK(e.longest_match() == 0);
    }
}

TEST_CASE("nstr::until", "[until]") {
    {
        std::string str1, str2;
        sstr ss("aaa,bbb");
        ss >> until(",", str1) >> str2;
        CHECK(str1 == "aaa");
        CHECK(str2 == "bbb");
    }
    {
        std::string str1, str2;
        sstr ss("aaa,   bbb");
        ss >> until(", *", str1) >> str2;
        CHECK(str1 == "aaa");
        CHECK(str2 == "bbb");
    }
    {
        std::string str1;
        sstr ss("aaa");
        CHECK_THROWS_AS(ss >> until(";", str1), invalid_input);
    }
    {
        std::string str1, str2;
        sstr ss("aaa,,,,,");
        ss >> until(",{1,2}", str1) >> str2;
        CHECK(str1 == "aaa");
        CHECK(str2 == ",,,");
    }
    {
        std::string str1, str2;
        sstr ss("aaa");
        ss >> until("b*", str1) >> str2;
        CHECK(str1 == "");
        CHECK(str2 == "aaa");
    }
    {
        std::string str1, str2;
        sstr ss("aaa;;;;;");
        ss >> until("(;;)+", str1) >> str2;
        CHECK(str1 == "aaa");
        CHECK(str2 == ";");
    }
}

TEST_CASE("nstr::all", "[all]") {
    {
        std::string str1, str2;
        sstr ss("abcabc defdef 123123 ...,,,,");
        ss >> all(str1) >> str2;
        CHECK(str1 == "abcabc defdef 123123 ...,,,,");
        CHECK(str2 == "");
        CHECK(ss.eof());
    }
}

TEST_CASE("nstr::split", "[split]") {
    {
        std::vector<int> vec, refvec = {10, 20, 30};
        sstr ss("10,20,30\n");
        std::string rest;
        ss >> split(",", "\n", vec) >> rest;
        CHECK(vec == refvec);
        CHECK(rest == "");
    }
    {
        std::vector<int> vec, refvec = {10, 20, 30};
        sstr ss("10,,,,20,,30;;;");
        std::string rest;
        ss >> split(",+", ";+", vec) >> rest;
        CHECK(vec == refvec);
        CHECK(rest == "");
    }
    {
        std::vector<int> vec, refvec = {10, 20, 30};
        sstr ss("10,;20,;;30,;;,");
        std::string rest;
        ss >> split(",;*", ",;;,", vec) >> rest;
        CHECK(vec == refvec);
        CHECK(rest == "");
    }
    {
        std::vector<std::string> vec,
            refvec = {"aa bb", "cc dd"};
        sstr ss("aa bb,cc dd\n");
        std::string rest;
        ss >> split(",", "\n", vec) >> rest;
        CHECK(vec == refvec);
        CHECK(rest == "");
    }
    {
        std::list<int> ls, refls = {10, 20, 30};
        sstr ss("10,20,30\n");
        std::string rest;
        ss >> split(",", "\n", ls) >> rest;
        CHECK(ls == refls);
        CHECK(rest == "");
    }
    {
        std::set<int> st, refst = {10, 20, 30};
        sstr ss("10,20,30\n");
        std::string rest;
        ss >> split(",", "\n", st) >> rest;
        CHECK(st == refst);
        CHECK(rest == "");
    }
    {
        std::vector<int> vec;
        sstr ss("10x,20;");
        CHECK_THROWS_AS(ss >> split(",", ";", vec), invalid_input);
    }
}
