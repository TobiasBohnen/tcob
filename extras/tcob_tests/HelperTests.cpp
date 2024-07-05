#include "tests.hpp"

#include <list>
#include <set>
#include <vector>

using namespace tcob::helper;

TEST_CASE("Core.Helper.LevenshteinDistance")
{
    REQUIRE(get_levenshtein_distance("xyu", "") == 3);
    REQUIRE(get_levenshtein_distance("", "xyu") == 3);

    REQUIRE(get_levenshtein_distance("ab", "abc") == 1);
    REQUIRE(get_levenshtein_distance("abc", "ab") == 1);

    REQUIRE(get_levenshtein_distance("stone", "sotne") == 2);
    REQUIRE(get_levenshtein_distance("sotne", "stone") == 2);

    REQUIRE(get_levenshtein_distance("car", "man") == 2);
    REQUIRE(get_levenshtein_distance("man", "car") == 2);

    REQUIRE(get_levenshtein_distance("book", "back") == 2);
    REQUIRE(get_levenshtein_distance("back", "book") == 2);

    REQUIRE(get_levenshtein_distance("kitten", "sitting") == 3);
    REQUIRE(get_levenshtein_distance("sitting", "kitten") == 3);

    REQUIRE(get_levenshtein_distance("book", "bookcase") == 4);
    REQUIRE(get_levenshtein_distance("bookcase", "book") == 4);

    REQUIRE(get_levenshtein_distance("bla", "bla") == 0);
    REQUIRE(get_levenshtein_distance("bla", "bla") == 0);

    REQUIRE(get_levenshtein_distance("money", "home") == 3);
    REQUIRE(get_levenshtein_distance("home", "money") == 3);
}

TEST_CASE("Core.Helper.JoinStrings")
{
    SUBCASE("array")
    {
        std::array<std::string, 4> col {"A", "B", "C", "D"};
        REQUIRE(helper::join(col, ",") == "A,B,C,D");
    }
    SUBCASE("vector")
    {
        std::vector<std::string> col {"A", "B", "C", "D"};
        REQUIRE(helper::join(col, ",") == "A,B,C,D");
    }
    SUBCASE("set")
    {
        std::set<std::string> col {"A", "B", "C", "D"};
        REQUIRE(helper::join(col, ",") == "A,B,C,D");
    }
    SUBCASE("list")
    {
        std::list<std::string> col {"A", "B", "C", "D"};
        REQUIRE(helper::join(col, ",") == "A,B,C,D");
    }
    SUBCASE("multichar delim")
    {
        std::list<std::string> col {"A", "B", "C", "D"};
        REQUIRE(helper::join(col, "--") == "A--B--C--D");
    }
}

TEST_CASE("Core.Helper.JoinNonStrings")
{
    SUBCASE("int")
    {
        std::array<int, 4> col {1, 2, 3, 4};
        REQUIRE(helper::join(col, ",") == "1,2,3,4");
    }
    SUBCASE("const char*")
    {
        std::array<char const*, 4> col {"1", "2", "3", "4"};
        REQUIRE(helper::join(col, ",") == "1,2,3,4");
    }
}

TEST_CASE("Core.Helper.SplitStrings")
{
    {
        std::string test = "1,2,3,4,5,6";
        REQUIRE(helper::split(test, ',') == std::vector<std::string> {"1", "2", "3", "4", "5", "6"});
    }
    {
        std::string test = "1, 2, 3, 4, 5, 6";
        REQUIRE(helper::split(test, ',') == std::vector<std::string> {"1", " 2", " 3", " 4", " 5", " 6"});
    }
    {
        std::string test = "1,2,3,4,5,";
        REQUIRE(helper::split(test, ',') == std::vector<std::string> {"1", "2", "3", "4", "5"});
    }
}
TEST_CASE("Core.Helper.SplitPreserveStrings")
{
    {
        std::string test = "1,2,{234,12},5";
        REQUIRE(helper::split_preserve_brackets(test, ',') == std::vector<std::string> {"1", "2", "{234,12}", "5"});
    }
    {
        std::string test   = "1,{123,{3,4,5,<6>}}";
        auto        result = helper::split_preserve_brackets(test, ',');
        REQUIRE(result == std::vector<std::string> {"1", "{123,{3,4,5,<6>}}"});
        result = helper::split_preserve_brackets(result[1], ',');
        REQUIRE(result == std::vector<std::string> {"{123,{3,4,5,<6>}}"});
        result = helper::split_preserve_brackets(result[0].substr(1, result[0].length() - 2), ',');
        REQUIRE(result == std::vector<std::string> {"123", "{3,4,5,<6>}"});
        result = helper::split_preserve_brackets(result[1].substr(1, result[1].length() - 2), ',');
        REQUIRE(result == std::vector<std::string> {"3", "4", "5", "<6>"});
    }
    {
        std::string test = "{1,2},\"3,4\",(5,6),<7,8>,[8,9]";
        REQUIRE(helper::split_preserve_brackets(test, ',') == std::vector<std::string> {"{1,2}", "\"3,4\"", "(5,6)", "<7,8>", "[8,9]"});
    }
    {
        std::string test = "{1,2},\"3,4\",(5,6),<7,8>,[8,9]";
        REQUIRE(helper::split_preserve_brackets(test, ',') == std::vector<std::string> {"{1,2}", "\"3,4\"", "(5,6)", "<7,8>", "[8,9]"});
    }
}
TEST_CASE("Core.Helper.WildcardMatch")
{
    SUBCASE("w/o wildcard")
    {
        REQUIRE(helper::wildcard_match("", ""));
        REQUIRE(helper::wildcard_match("a", "a"));

        REQUIRE_FALSE(helper::wildcard_match("", "a"));
        REQUIRE_FALSE(helper::wildcard_match("a", ""));
    }

    SUBCASE("*")
    {
        REQUIRE(helper::wildcard_match("xyz..txt", "*.txt"));
        REQUIRE(helper::wildcard_match("xyyz", "x*z"));
        REQUIRE(helper::wildcard_match("xyyz", "x*z*"));
        REQUIRE(helper::wildcard_match("xyz.txt", "*.txt"));
        REQUIRE(helper::wildcard_match("xyz.txt.bla", "*.txt*"));
        REQUIRE(helper::wildcard_match("xyz.assets.txt", "*.assets.*"));
        REQUIRE(helper::wildcard_match("xyz.assets..txt", "*.assets.*"));
        REQUIRE(helper::wildcard_match("xyz", "*"));
        REQUIRE(helper::wildcard_match("xyz", "**"));
        REQUIRE(helper::wildcard_match("xyz", "***"));
        REQUIRE(helper::wildcard_match("xyz", "****"));
        REQUIRE(helper::wildcard_match("abc", "a*c"));
        REQUIRE(helper::wildcard_match("abcd", "a*d"));
        REQUIRE(helper::wildcard_match("abcd", "a*c*d"));
        REQUIRE(helper::wildcard_match("abcd", "*"));
        REQUIRE(helper::wildcard_match("abcd", "*d"));
        REQUIRE(helper::wildcard_match("abcd", "a*"));
        REQUIRE(helper::wildcard_match("abcd", "a**d"));
        REQUIRE(helper::wildcard_match("", "*"));
        REQUIRE(helper::wildcard_match("abcde", "a*e"));
        REQUIRE(helper::wildcard_match("abcde", "*bc*e*"));

        REQUIRE_FALSE(helper::wildcard_match("xyyb", "x*z"));
        REQUIRE_FALSE(helper::wildcard_match("xyyb", "x*z*"));
        REQUIRE_FALSE(helper::wildcard_match("xyz.txt.bla", "*.txt"));
        REQUIRE_FALSE(helper::wildcard_match("xyz.txt", "*.lua"));
        REQUIRE_FALSE(helper::wildcard_match("abc", "a*d"));
        REQUIRE_FALSE(helper::wildcard_match("abc", "a*b*d"));
        REQUIRE_FALSE(helper::wildcard_match("abc", "*a"));
        REQUIRE_FALSE(helper::wildcard_match("abc", "b*"));
        REQUIRE_FALSE(helper::wildcard_match("abcde", "a*c"));
    }
    SUBCASE("?")
    {
        REQUIRE(helper::wildcard_match("x", "?"));
        REQUIRE(helper::wildcard_match("xyz", "x?z"));
        REQUIRE(helper::wildcard_match("a.txt", "?.txt"));
        REQUIRE(helper::wildcard_match("aaa.txt", "???.txt"));
        REQUIRE(helper::wildcard_match("a", "?"));

        REQUIRE_FALSE(helper::wildcard_match("xyyb", "x?b"));
        REQUIRE_FALSE(helper::wildcard_match("xyz.txt", "?.txt"));
        REQUIRE_FALSE(helper::wildcard_match("x", "??"));
    }
    SUBCASE("*?")
    {
        REQUIRE(helper::wildcard_match("xyyza", "x*z?"));
        REQUIRE(helper::wildcard_match("abc", "a*?"));
        REQUIRE(helper::wildcard_match("abcd", "a*c?"));
        REQUIRE(helper::wildcard_match("a", "*?"));
        REQUIRE(helper::wildcard_match("a", "?*"));

        REQUIRE_FALSE(helper::wildcard_match("abc", "a*c?"));
        REQUIRE_FALSE(helper::wildcard_match("xyyz", "x*z?"));
    }
}

TEST_CASE("Core.Math.PointOnLine")
{
    {
        point_f p0 {0, 0};
        point_f p1 {3, 3};
        REQUIRE(point_f::Lerp(p0, p1, 0.5f) == point_f {1.5f, 1.5f});
    }
    {
        point_f p0 {10.f, 10.f};
        point_f p1 {2.f, 5.f};
        REQUIRE(point_f::Lerp(p0, p1, 0.1f) == point_f {9.2f, 9.5f});
    }
}

TEST_CASE("Core.Math.PointOnRectangle")
{
    using namespace tcob::literals;

    {
        rect_f rect {0, 0, 10, 10};
        REQUIRE_MESSAGE(rect.find_edge(0_deg).equals({10.f, 5.f}, 0.001f), rect.find_edge(0_deg));
        REQUIRE_MESSAGE(rect.find_edge(45_deg).equals({10.f, 10.f}, 0.001f), rect.find_edge(45_deg));
        REQUIRE_MESSAGE(rect.find_edge(90_deg).equals({5.f, 10.f}, 0.001f), rect.find_edge(90_deg));
        REQUIRE_MESSAGE(rect.find_edge(135_deg).equals({0.f, 10.f}, 0.001f), rect.find_edge(135_deg));
        REQUIRE_MESSAGE(rect.find_edge(180_deg).equals({0.f, 5.f}, 0.001f), rect.find_edge(180_deg));
        REQUIRE_MESSAGE(rect.find_edge(225_deg).equals({0.f, 0.f}, 0.001f), rect.find_edge(225_deg));
        REQUIRE_MESSAGE(rect.find_edge(270_deg).equals({5.f, 0.f}, 0.001f), rect.find_edge(270_deg));
        REQUIRE_MESSAGE(rect.find_edge(315_deg).equals({10.f, 0.f}, 0.001f), rect.find_edge(315_deg));
        REQUIRE_MESSAGE(rect.find_edge(360_deg).equals({10.f, 5.f}, 0.001f), rect.find_edge(360_deg));
    }
}

TEST_CASE("Core.Helper.RoundToMultiple")
{
    SUBCASE("5")
    {
        REQUIRE(helper::round_to_multiple(-11, 5) == -10);
        REQUIRE(helper::round_to_multiple(-10, 5) == -10);
        REQUIRE(helper::round_to_multiple(-9, 5) == -10);
        REQUIRE(helper::round_to_multiple(-8, 5) == -10);
        REQUIRE(helper::round_to_multiple(-7, 5) == -5);
        REQUIRE(helper::round_to_multiple(-6, 5) == -5);
        REQUIRE(helper::round_to_multiple(-5, 5) == -5);
        REQUIRE(helper::round_to_multiple(-4, 5) == -5);
        REQUIRE(helper::round_to_multiple(-3, 5) == -5);
        REQUIRE(helper::round_to_multiple(-2, 5) == 0);
        REQUIRE(helper::round_to_multiple(-1, 5) == 0);
        REQUIRE(helper::round_to_multiple(0, 5) == 0);
        REQUIRE(helper::round_to_multiple(1, 5) == 0);
        REQUIRE(helper::round_to_multiple(2, 5) == 0);
        REQUIRE(helper::round_to_multiple(3, 5) == 5);
        REQUIRE(helper::round_to_multiple(4, 5) == 5);
        REQUIRE(helper::round_to_multiple(5, 5) == 5);
        REQUIRE(helper::round_to_multiple(6, 5) == 5);
        REQUIRE(helper::round_to_multiple(7, 5) == 5);
        REQUIRE(helper::round_to_multiple(8, 5) == 10);
        REQUIRE(helper::round_to_multiple(9, 5) == 10);
        REQUIRE(helper::round_to_multiple(10, 5) == 10);
        REQUIRE(helper::round_to_multiple(11, 5) == 10);
    }

    SUBCASE("4")
    {
        REQUIRE(helper::round_to_multiple(-11, 4) == -12);
        REQUIRE(helper::round_to_multiple(-10, 4) == -8);
        REQUIRE(helper::round_to_multiple(-9, 4) == -8);
        REQUIRE(helper::round_to_multiple(-8, 4) == -8);
        REQUIRE(helper::round_to_multiple(-7, 4) == -8);
        REQUIRE(helper::round_to_multiple(-6, 4) == -4);
        REQUIRE(helper::round_to_multiple(-5, 4) == -4);
        REQUIRE(helper::round_to_multiple(-4, 4) == -4);
        REQUIRE(helper::round_to_multiple(-3, 4) == -4);
        REQUIRE(helper::round_to_multiple(-2, 4) == 0);
        REQUIRE(helper::round_to_multiple(-1, 4) == 0);
        REQUIRE(helper::round_to_multiple(0, 4) == 0);
        REQUIRE(helper::round_to_multiple(1, 4) == 0);
        REQUIRE(helper::round_to_multiple(2, 4) == 0);
        REQUIRE(helper::round_to_multiple(3, 4) == 4);
        REQUIRE(helper::round_to_multiple(4, 4) == 4);
        REQUIRE(helper::round_to_multiple(5, 4) == 4);
        REQUIRE(helper::round_to_multiple(6, 4) == 4);
        REQUIRE(helper::round_to_multiple(7, 4) == 8);
        REQUIRE(helper::round_to_multiple(8, 4) == 8);
        REQUIRE(helper::round_to_multiple(9, 4) == 8);
        REQUIRE(helper::round_to_multiple(10, 4) == 8);
        REQUIRE(helper::round_to_multiple(11, 4) == 12);
    }
}

TEST_CASE("Core.Helper.RoundUpToMultiple")
{
    SUBCASE("5")
    {
        REQUIRE(helper::round_up_to_multiple(-11, 5) == -10);
        REQUIRE(helper::round_up_to_multiple(-10, 5) == -10);
        REQUIRE(helper::round_up_to_multiple(-9, 5) == -5);
        REQUIRE(helper::round_up_to_multiple(-8, 5) == -5);
        REQUIRE(helper::round_up_to_multiple(-7, 5) == -5);
        REQUIRE(helper::round_up_to_multiple(-6, 5) == -5);
        REQUIRE(helper::round_up_to_multiple(-5, 5) == -5);
        REQUIRE(helper::round_up_to_multiple(-4, 5) == 0);
        REQUIRE(helper::round_up_to_multiple(-3, 5) == 0);
        REQUIRE(helper::round_up_to_multiple(-2, 5) == 0);
        REQUIRE(helper::round_up_to_multiple(-1, 5) == 0);
        REQUIRE(helper::round_up_to_multiple(0, 5) == 0);
        REQUIRE(helper::round_up_to_multiple(1, 5) == 5);
        REQUIRE(helper::round_up_to_multiple(2, 5) == 5);
        REQUIRE(helper::round_up_to_multiple(3, 5) == 5);
        REQUIRE(helper::round_up_to_multiple(4, 5) == 5);
        REQUIRE(helper::round_up_to_multiple(5, 5) == 5);
        REQUIRE(helper::round_up_to_multiple(6, 5) == 10);
        REQUIRE(helper::round_up_to_multiple(7, 5) == 10);
        REQUIRE(helper::round_up_to_multiple(8, 5) == 10);
        REQUIRE(helper::round_up_to_multiple(9, 5) == 10);
        REQUIRE(helper::round_up_to_multiple(10, 5) == 10);
        REQUIRE(helper::round_up_to_multiple(11, 5) == 15);
    }

    SUBCASE("4")
    {
        REQUIRE(helper::round_up_to_multiple(-11, 4) == -8);
        REQUIRE(helper::round_up_to_multiple(-10, 4) == -8);
        REQUIRE(helper::round_up_to_multiple(-9, 4) == -8);
        REQUIRE(helper::round_up_to_multiple(-8, 4) == -8);
        REQUIRE(helper::round_up_to_multiple(-7, 4) == -4);
        REQUIRE(helper::round_up_to_multiple(-6, 4) == -4);
        REQUIRE(helper::round_up_to_multiple(-5, 4) == -4);
        REQUIRE(helper::round_up_to_multiple(-4, 4) == -4);
        REQUIRE(helper::round_up_to_multiple(-3, 4) == 0);
        REQUIRE(helper::round_up_to_multiple(-2, 4) == 0);
        REQUIRE(helper::round_up_to_multiple(-1, 4) == 0);
        REQUIRE(helper::round_up_to_multiple(0, 4) == 0);
        REQUIRE(helper::round_up_to_multiple(1, 4) == 4);
        REQUIRE(helper::round_up_to_multiple(2, 4) == 4);
        REQUIRE(helper::round_up_to_multiple(3, 4) == 4);
        REQUIRE(helper::round_up_to_multiple(4, 4) == 4);
        REQUIRE(helper::round_up_to_multiple(5, 4) == 8);
        REQUIRE(helper::round_up_to_multiple(6, 4) == 8);
        REQUIRE(helper::round_up_to_multiple(7, 4) == 8);
        REQUIRE(helper::round_up_to_multiple(8, 4) == 8);
        REQUIRE(helper::round_up_to_multiple(9, 4) == 12);
        REQUIRE(helper::round_up_to_multiple(10, 4) == 12);
        REQUIRE(helper::round_up_to_multiple(11, 4) == 12);
    }
}
