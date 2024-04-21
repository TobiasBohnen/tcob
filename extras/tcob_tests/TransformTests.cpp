#include "tests.hpp"

TEST_CASE("Core.POD.Transform")
{
    SUBCASE("translate")
    {
        transform t;
        t.translate({1.f, 2.f});
        REQUIRE(t.Matrix[6] == Approx(1.f));
        REQUIRE(t.Matrix[7] == Approx(2.f));
    }
    SUBCASE("rotate")
    {
        transform t;
        t.rotate(90.f);
        REQUIRE(t.Matrix[0] == Approx(0.f));
        REQUIRE(t.Matrix[1] == Approx(1.f));
        REQUIRE(t.Matrix[2] == Approx(0.f));
        REQUIRE(t.Matrix[3] == Approx(-1.f));
        REQUIRE(t.Matrix[4] == Approx(0.f));
        REQUIRE(t.Matrix[5] == Approx(0.f));
        REQUIRE(t.Matrix[6] == Approx(0.f));
        REQUIRE(t.Matrix[7] == Approx(0.f));
        REQUIRE(t.Matrix[8] == Approx(1.f));
    }
    SUBCASE("transform_point")
    {
        {
            transform t;
            t.rotate({90.f});
            auto p = t.transform_point({2.f, 3.f});
            REQUIRE(p.X == Approx(-3.f));
            REQUIRE(p.Y == Approx(2.f));
        }
        {
            transform t;
            t.translate({3.f, -2.f});
            t.rotate({90.f});
            auto p = t.transform_point({2.f, 3.f});
            REQUIRE(p.X == Approx(0.f));
            REQUIRE(p.Y == Approx(0.f));
        }
    }
    SUBCASE("is_translate_only")
    {
        transform good0;
        good0.translate({1.f, 2.f});
        REQUIRE(good0.is_translate_only());
        good0.translate({5.f, -2.f});
        REQUIRE(good0.is_translate_only());
        good0.translate({1.23f, 4.63f});
        REQUIRE(good0.is_translate_only());

        transform good1 {good0};
        good1.scale_at({1, 1}, {4, 5});
        REQUIRE(good1.is_translate_only());

        transform bad0 {good0};
        bad0.rotate({90.f});
        REQUIRE_FALSE(bad0.is_translate_only());

        transform bad1 {good0};
        bad1.rotate_at({35.f}, {20, 30});
        REQUIRE_FALSE(bad1.is_translate_only());

        transform bad2 {good0};
        bad2.skew({90.f, 10});
        REQUIRE_FALSE(bad2.is_translate_only());
    }
}
