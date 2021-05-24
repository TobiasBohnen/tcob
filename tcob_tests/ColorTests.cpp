#include "tests.hpp"

TEST_CASE("Core.Data.Color")
{
    static_assert(std::is_copy_constructible_v<Color>);
    static_assert(std::is_copy_assignable_v<Color>);
    static_assert(std::is_move_constructible_v<Color>);
    static_assert(std::is_move_assignable_v<Color>);

    SECTION("Construction")
    {
        {
            Color p;
            REQUIRE(p.A == 0);
            REQUIRE(p.R == 0);
            REQUIRE(p.G == 0);
            REQUIRE(p.B == 0);
        }
        {
            Color p { 10, 20, 30, 40 };
            REQUIRE(p.R == 10);
            REQUIRE(p.G == 20);
            REQUIRE(p.B == 30);
            REQUIRE(p.A == 40);
        }
        {
            Color p1 { 10, 20, 30, 40 };
            Color p2 { p1 };
            REQUIRE(p2.R == 10);
            REQUIRE(p2.G == 20);
            REQUIRE(p2.B == 30);
            REQUIRE(p2.A == 40);
        }
        {
            u32 col { 0xffeeddcc };
            Color p { col };
            REQUIRE(p.R == 0xff);
            REQUIRE(p.G == 0xee);
            REQUIRE(p.B == 0xdd);
            REQUIRE(p.A == 0xcc);
        }
    }

    SECTION("Equality")
    {
        {
            Color p1 { 10, 20, 30, 40 };
            Color p2 { 10, 20, 30, 40 };
            REQUIRE(p1 == p2);
        }
        {
            Color p1 { Colors::Beige };
            Color p2 { 0xF5F5DCFF };
            REQUIRE(p1 == p2);
        }
        {
            Color p1 { 10, 20, 30, 40 };
            Color p2 { 40, 30, 20, 10 };
            REQUIRE_FALSE(p1 == p2);
        }
    }

    SECTION("Interpolation")
    {
        Color pexp { 0x7f, 0x7f, 0x7f, 0xFF };
        Color p1 { 0, 0, 0, 0xFF };
        Color p2 { 0xFF, 0xFF, 0xFF, 0xFF };
        Color pact = p1.interpolate(p2, 0.5);
        REQUIRE(pact == pexp);
    }

    SECTION("PreMultiplyAlpha")
    {
        Color pexp { 25, 50, 100, 0x80 };
        Color p1 { 50, 100, 200, 0x80 };
        Color pact { p1.premultiply_alpha() };
        REQUIRE(pact == pexp);
    }
    SECTION("FromString")
    {
        REQUIRE(Colors::FromString("Red") == Colors::Red);
        REQUIRE(Colors::FromString("Blue") == Colors::Blue);
        REQUIRE(Colors::FromString("RebeccaPurple") == Colors::RebeccaPurple);

        REQUIRE(Colors::FromString("#FF00FF") == Color { 255, 0, 255, 255 });
        REQUIRE(Colors::FromString("#01020304") == Color { 1, 2, 3, 4 });
    }
}