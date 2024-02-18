#include "tests.hpp"

TEST_CASE("Core.FlatMap.Basic")
{
    flat_map<string, u32> test;
    test["a"] = 100;
    test["b"] = 200;
    test["c"] = 300;

    SUBCASE("contains")
    {
        REQUIRE(test.contains("a"));
    }

    SUBCASE("at")
    {
        REQUIRE(test.at("a") == 100);
        REQUIRE_THROWS(test.at("xxx"));
    }

    SUBCASE("clear/empty/size")
    {
        REQUIRE(test.size() == 3);
        test.clear();
        REQUIRE(test.size() == 0);
        REQUIRE(test.empty());
    }

    SUBCASE("find")
    {
        auto it {test.find("b")};
        REQUIRE(it != test.end());
        REQUIRE(it->second == 200);
    }

    SUBCASE("erase")
    {
        REQUIRE(test.size() == 3);
        REQUIRE(test.contains("b"));
        test.erase("b");
        REQUIRE(test.size() == 2);
        REQUIRE_FALSE(test.contains("b"));
    }

    SUBCASE("erase_if")
    {
        REQUIRE(test.size() == 3);
        test.erase_if([](auto p) { return p.second > 100; });
        REQUIRE(test.size() == 1);
        REQUIRE(test.contains("a"));
    }
}
