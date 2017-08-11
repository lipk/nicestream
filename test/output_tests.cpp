#include <catch.hpp>
#include <sstream>

#include <nicestream.hpp>

using namespace nstr;

typedef std::stringstream sstr;

TEST_CASE("nstr::join", "[join]") {
    {
        std::vector<int> vec = {1, 2, 3, 4};
        std::string res;
        sstr ss(res);
        ss << join(", ", vec) << std::flush;
        CHECK(ss.str() == "1, 2, 3, 4");
    }
    {
        std::set<char> st = {'a', 'c', ' ', 'b'};
        std::string res;
        sstr ss(res);
        ss << join("", st) << std::flush;
        CHECK(ss.str() == " abc");
    }
    {
        int arr[] = {3, 4, 5};
        std::string res;
        sstr ss(res);
        ss << join(", ", arr, arr + 3) << std::flush;
        CHECK(ss.str() == "3, 4, 5");
    }
}
