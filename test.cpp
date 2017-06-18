#define CATCH_CONFIG_MAIN
#include <catch.hpp>
#include <string>
#include <sstream>

#include "nicestream.hpp"

using namespace nstr;

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
