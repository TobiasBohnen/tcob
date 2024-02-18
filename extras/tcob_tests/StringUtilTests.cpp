#include "tests.hpp"

TEST_CASE("Core.StringUtils.Utf8")
{
    SUBCASE("remove")
    {
        utf8_string s0 {"ä123äabc"};
        REQUIRE(utf8::remove(s0, 0, 3) == "3äabc");
        REQUIRE(utf8::remove(s0, 1, 3) == "ääabc");
        REQUIRE(utf8::remove(s0, 2, 3) == "ä1abc");
        REQUIRE(utf8::remove(s0, 3, 3) == "ä12bc");
    }
    SUBCASE("substr")
    {
        utf8_string s0 {"ä123äabc"};
        REQUIRE(utf8::substr(s0, 0, 3) == "ä12");
        REQUIRE(utf8::substr(s0, 1, 3) == "123");
        REQUIRE(utf8::substr(s0, 2, 3) == "23ä");
        REQUIRE(utf8::substr(s0, 3, 3) == "3äa");
    }
}
