#include "tcob/core/AngleUnits.hpp"
#include "tests.hpp"

TEST_CASE("Core.POD.AngleUnits")
{
    SUBCASE("radian to degree")
    {
        {
            auto rad = radian_f {0.0174533f};
            auto deg = degree_f {rad};
            REQUIRE(deg.Value == Approx(1.f));
        }
        {
            auto deg = degree_f {180};
            auto rad = radian_f {deg};
            REQUIRE(rad.Value == Approx(TAU / 2));
        }
    }

    SUBCASE("radian to turn")
    {
        {
            auto rad = radian_f {TAU_F};
            auto tur = turn_f {rad};
            REQUIRE(tur.Value == Approx(1.f));
        }
        {
            auto tur = turn_f {0.25f};
            auto rad = radian_f {tur};
            REQUIRE(rad.Value == Approx(TAU / 4));
        }
    }
    SUBCASE("radian to gradian")
    {
        {
            auto rad = radian_f {0.0174533f};
            auto gon = gradian_f {rad};
            REQUIRE(gon.Value == Approx(1.111111f));
        }
        {
            auto gon = gradian_f {200};
            auto rad = radian_f {gon};
            REQUIRE(rad.Value == Approx(TAU / 2));
        }
    }
    SUBCASE("degree to turn")
    {
        {
            auto deg = degree_f {180};
            auto tur = turn_f {deg};
            REQUIRE(tur.Value == Approx(0.5f));
        }
        {
            auto tur = turn_f {0.25f};
            auto deg = degree_f {tur};
            REQUIRE(deg.Value == Approx(90));
        }
    }
    SUBCASE("degree to gradian")
    {
        {
            auto deg = degree_f {90};
            auto gon = gradian_f {deg};
            REQUIRE(gon.Value == Approx(100));
        }
        {
            auto gon = gradian_f {200};
            auto deg = degree_f {gon};
            REQUIRE(deg.Value == Approx(180));
        }
    }
    SUBCASE("gradian to turn")
    {
        {
            auto gon = gradian_f {200};
            auto tur = turn_f {gon};
            REQUIRE(tur.Value == Approx(0.5f));
        }
        {
            auto tur = turn_f {0.25f};
            auto gon = gradian_f {tur};
            REQUIRE(gon.Value == Approx(100));
        }
    }
    SUBCASE("float to double")
    {
        {
            auto degf = degree_f {180};
            auto degd = degree_d {degf};
            REQUIRE(degf.Value == Approx(degd.Value));
        }
        {
            auto radf = radian_f {0.0174533f};
            auto radd = radian_d {radf};
            REQUIRE(radf.Value == Approx(radd.Value));
        }
    }
    SUBCASE("Lerp")
    {
        {
            auto deg0 = degree_f {90};
            auto deg1 = degree_f {270};
            REQUIRE(degree_f::Lerp(deg0, deg1, 0.5f) == degree_f(180));
        }
    }
    SUBCASE("normalize")
    {
        {
            REQUIRE((degree_f {10} - degree_f {90}).as_normalized() == degree_f {-80});
            REQUIRE((degree_f {350} + degree_f {90}).as_normalized() == degree_f {80});
            REQUIRE((degree_f {350} * 3).as_normalized() == degree_f {330});
            REQUIRE((degree_f {720}).as_normalized() == degree_f {0});
            REQUIRE((degree_f {-720}).as_normalized() == degree_f {0});
        }
        {
            REQUIRE((radian_f {TAU_F} * 3).as_normalized().Value == Approx(radian_f {TAU_F}.Value));
            REQUIRE((radian_f {TAU_F} + radian_f {TAU_F / 2}).as_normalized().Value == Approx(radian_f {TAU_F / 2}.Value));
        }
        {
            REQUIRE((turn_f {0.1f} - turn_f {0.8f}).as_normalized() == turn_f {-0.7f});
            REQUIRE((turn_f {0.25f} * 6).as_normalized() == turn_f {0.5f});
        }
        {
            REQUIRE((gradian_f {10} - gradian_f {90}).as_normalized() == gradian_f {-80});
            REQUIRE((gradian_f {350} + gradian_f {90}).as_normalized() == gradian_f {40});
            REQUIRE((gradian_f {350} * 3).as_normalized() == gradian_f {250});
            REQUIRE((gradian_f {800}).as_normalized() == gradian_f {0});
            REQUIRE((gradian_f {-800}).as_normalized() == gradian_f {0});
        }
    }
    SUBCASE("literals")
    {
        using namespace tcob::literals;
        {
            REQUIRE(90_deg + 270_deg == degree_f {360});
            REQUIRE(90_rad + 270_rad == radian_f {360});
            REQUIRE(90_turn + 270_turn == turn_f {360});
            REQUIRE(90_grad + 270_grad == gradian_f {360});
        }
    }
}
