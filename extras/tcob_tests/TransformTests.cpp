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
}
