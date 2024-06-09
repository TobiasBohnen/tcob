#include "tests.hpp"

TEST_CASE("Core.Random.MinMax")
{
    SUBCASE("default")
    {
        rng rnd;
        SUBCASE("i32")
        {
            {
                i32 min = 8, max = 15;
                for (i32 i = 0; i < 5000; i++) {
                    auto x = rnd(min, max);
                    REQUIRE(x >= min);
                    REQUIRE(x <= max);
                }
            }
            {
                i32 min = -10, max = 15;
                for (i32 i = 0; i < 5000; i++) {
                    auto x = rnd(min, max);
                    REQUIRE(x >= min);
                    REQUIRE(x <= max);
                }
            }
            {
                i32 min = -5, max = -4;
                for (i32 i = 0; i < 5000; i++) {
                    auto x = rnd(min, max);
                    REQUIRE(x >= min);
                    REQUIRE(x <= max);
                }
            }
        }
        SUBCASE("float")
        {
            {
                float min = 8.f, max = 15.f;
                for (i32 i = 0; i < 5000; i++) {
                    auto x = rnd(min, max);
                    REQUIRE(x >= min);
                    REQUIRE(x <= max);
                }
            }
            {
                float min = -10.f, max = 15.f;
                for (i32 i = 0; i < 5000; i++) {
                    auto x = rnd(min, max);
                    REQUIRE(x >= min);
                    REQUIRE(x <= max);
                }
            }
            {
                float min = -5.f, max = -4.f;
                for (i32 i = 0; i < 5000; i++) {
                    auto x = rnd(min, max);
                    REQUIRE(x >= min);
                    REQUIRE(x <= max);
                }
            }
        }
    }
    SUBCASE("split_mix_32")
    {
        random::rng_split_mix_32 rnd;
        SUBCASE("i32")
        {
            {
                i32 min = 8, max = 15;
                for (i32 i = 0; i < 5000; i++) {
                    auto x = rnd(min, max);
                    REQUIRE(x >= min);
                    REQUIRE(x <= max);
                }
            }
            {
                i32 min = -10, max = 15;
                for (i32 i = 0; i < 5000; i++) {
                    auto x = rnd(min, max);
                    REQUIRE(x >= min);
                    REQUIRE(x <= max);
                }
            }
            {
                i32 min = -5, max = -4;
                for (i32 i = 0; i < 5000; i++) {
                    auto x = rnd(min, max);
                    REQUIRE(x >= min);
                    REQUIRE(x <= max);
                }
            }
        }
        SUBCASE("float")
        {
            {
                float min = 8.f, max = 15.f;
                for (i32 i = 0; i < 5000; i++) {
                    auto x = rnd(min, max);
                    REQUIRE(x >= min);
                    REQUIRE(x <= max);
                }
            }
            {
                float min = -10.f, max = 15.f;
                for (i32 i = 0; i < 5000; i++) {
                    auto x = rnd(min, max);
                    REQUIRE(x >= min);
                    REQUIRE(x <= max);
                }
            }
            {
                float min = -5.f, max = -4.f;
                for (i32 i = 0; i < 5000; i++) {
                    auto x = rnd(min, max);
                    REQUIRE(x >= min);
                    REQUIRE(x <= max);
                }
            }
        }
    }
}

TEST_CASE("Core.Random.CopyState")
{
    rng r0 {12345};
    for (i32 i = 0; i < 1000; i++) {
        r0(0, 1);
    }

    rng r1 {r0.get_state()};
    i32 min = 8, max = 15;
    for (i32 i = 0; i < 1000; i++) {
        REQUIRE(r0(min, max) == r1(min, max));
    }
}

TEST_CASE("Core.Random.Copy")
{
    {
        rng r0 {12345};
        for (i32 i = 0; i < 1000; i++) {
            r0(0, 1);
        }

        rng r1 {r0};
        i32 min = 8, max = 15;
        for (i32 i = 0; i < 1000; i++) {
            REQUIRE(r0(min, max) == r1(min, max));
        }
    }
    {
        random::dice<6> r0 {12345};
        for (i32 i = 0; i < 1000; i++) {
            r0.roll();
        }
        random::dice<6> r1 {r0};
        for (i32 i = 0; i < 1000; i++) {
            REQUIRE(r0.roll() == r1.roll());
        }
        random::dice<6> r2 {};
        r2 = r1;
        for (i32 i = 0; i < 1000; i++) {
            i32 roll = r1.roll();
            REQUIRE(r0.roll() == roll);
            REQUIRE(r2.roll() == roll);
        }
    }
}

TEST_CASE("Core.Random.Seq")
{
    {
        rng              r0 {50413220};
        std::vector<i32> v {};
        v.reserve(10);
        for (i32 i = 0; i < 10; i++) {
            v.push_back(r0(0, 10));
        }

        REQUIRE(v == std::vector<i32> {4, 6, 7, 9, 4, 9, 10, 0, 4, 5});
    }
    {
        rng              r0 {54321};
        std::vector<i32> v {};
        v.reserve(10);
        for (i32 i = 0; i < 10; i++) {
            v.push_back(r0(0, 100));
        }

        REQUIRE(v == std::vector<i32> {70, 82, 59, 22, 51, 58, 49, 47, 59, 31});
    }
}

TEST_CASE("Core.Random.Shuffle")
{
    {
        std::vector<i32>     v {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
        random::shuffle<i32> r0 {54321};
        r0(v);
        REQUIRE(v == std::vector<i32> {8, 5, 2, 4, 0, 3, 7, 6, 1, 9});
    }
    {
        std::array<i32, 10>  v {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
        random::shuffle<i32> r0 {54321};
        r0(v);
        REQUIRE(v == std::array<i32, 10> {8, 5, 2, 4, 0, 3, 7, 6, 1, 9});
    }
}
