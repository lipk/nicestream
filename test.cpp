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

    // ?, *, +
    CHECK_NOTHROW(sep("x?"));
    CHECK_NOTHROW(sep("(xy)?"));
    //CHECK_NOTHROW(sep("x.?"));
    CHECK_THROWS_AS(sep("?zzz"), invalid_regex);
    CHECK_NOTHROW(sep("x*"));
    CHECK_NOTHROW(sep("(xy)*"));
    //CHECK_NOTHROW(sep("x.*"));
    CHECK_THROWS_AS(sep("*zzz"), invalid_regex);
    CHECK_NOTHROW(sep("x+"));
    CHECK_NOTHROW(sep("(xy)+"));
    //CHECK_NOTHROW(sep("x.+"));
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

    // character classes
}
