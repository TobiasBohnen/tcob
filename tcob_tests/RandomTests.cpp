#include "tests.hpp"
#include <random>

TEST_CASE("Core.Random.MinMax")
{
    Random r;
    SECTION("int")
    {
        {
            int min = 8, max = 15;
            for (int i = 0; i < 1000; i++) {
                auto x = r(min, max);
                REQUIRE(x >= min);
                REQUIRE(x <= max);
            }
        }
        {
            int min = -10, max = 15;
            for (int i = 0; i < 1000; i++) {
                auto x = r(min, max);
                REQUIRE(x >= min);
                REQUIRE(x <= max);
            }
        }
        {
            int min = -5, max = -4;
            for (int i = 0; i < 1000; i++) {
                auto x = r(min, max);
                REQUIRE(x >= min);
                REQUIRE(x <= max);
            }
        }
    }
    SECTION("float")
    {
        {
            float min = 8.f, max = 15.f;
            for (int i = 0; i < 1000; i++) {
                auto x = r(min, max);
                REQUIRE(x >= min);
                REQUIRE(x <= max);
            }
        }
        {
            float min = -10.f, max = 15.f;
            for (int i = 0; i < 1000; i++) {
                auto x = r(min, max);
                REQUIRE(x >= min);
                REQUIRE(x <= max);
            }
        }
        {
            float min = -5.f, max = -4.f;
            for (int i = 0; i < 1000; i++) {
                auto x = r(min, max);
                REQUIRE(x >= min);
                REQUIRE(x <= max);
            }
        }
    }
}

TEST_CASE("Core.Random.Dist")
{
    Xoroshiro128Plus r;

    int min = 8, max = 15;
    std::uniform_int_distribution<> distrib(min, max);
    for (int i = 0; i < 1000; i++) {
        auto x = distrib(r);
        REQUIRE(x >= min);
        REQUIRE(x <= max);
    }
}

TEST_CASE("Core.Random.Equality")
{
    Random r0 { 12345 };
    Random r1 { r0 };
    int min = 8, max = 15;
    for (int i = 0; i < 1000; i++) {
        REQUIRE(r0(min, max) == r1(min, max));
    }
}