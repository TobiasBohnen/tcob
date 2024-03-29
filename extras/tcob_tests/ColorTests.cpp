#include "tests.hpp"

TEST_CASE("Core.POD.Color")
{
    static_assert(std::is_copy_constructible_v<color>);
    static_assert(std::is_copy_assignable_v<color>);
    static_assert(std::is_move_constructible_v<color>);
    static_assert(std::is_move_assignable_v<color>);
    static_assert(POD<color>);

    SUBCASE("Construction")
    {
        {
            color p;
            REQUIRE(p.A == 0);
            REQUIRE(p.R == 0);
            REQUIRE(p.G == 0);
            REQUIRE(p.B == 0);
        }
        {
            color p {10, 20, 30, 40};
            REQUIRE(p.R == 10);
            REQUIRE(p.G == 20);
            REQUIRE(p.B == 30);
            REQUIRE(p.A == 40);
        }
        {
            color p1 {10, 20, 30, 40};
            color p2 {p1};
            REQUIRE(p2.R == 10);
            REQUIRE(p2.G == 20);
            REQUIRE(p2.B == 30);
            REQUIRE(p2.A == 40);
        }
        {
            u32   col {0xffeeddcc};
            color p {color::FromRGBA(col)};
            REQUIRE(p.R == 0xff);
            REQUIRE(p.G == 0xee);
            REQUIRE(p.B == 0xdd);
            REQUIRE(p.A == 0xcc);
            REQUIRE(p.value() == col);
        }
    }

    SUBCASE("Equality")
    {
        {
            color p1 {10, 20, 30, 40};
            color p2 {10, 20, 30, 40};
            REQUIRE(p1 == p2);
        }
        {
            color p1 {colors::Beige};
            color p2 {color::FromRGBA(p1.value())};
            REQUIRE(p1 == p2);
        }
        {
            color p1 {10, 20, 30, 40};
            color p2 {40, 30, 20, 10};
            REQUIRE_FALSE(p1 == p2);
        }
    }

    SUBCASE("Lerp")
    {
        color pexp {0x7f, 0x7f, 0x7f, 0xFF};
        color p1 {0, 0, 0, 0xFF};
        color p2 {0xFF, 0xFF, 0xFF, 0xFF};
        color pact = color::Lerp(p1, p2, 0.5);
        REQUIRE(pact == pexp);
    }

    SUBCASE("PreMultiplyAlpha")
    {
        color pexp {25, 50, 100, 0x80};
        color p1 {50, 100, 200, 0x80};
        color pact {p1.as_alpha_premultiplied()};
        REQUIRE(pact == pexp);
    }

    SUBCASE("FromString")
    {
        REQUIRE(color::FromString("Red") == colors::Red);
        REQUIRE(color::FromString("red") == colors::Red);
        REQUIRE(color::FromString("rEd") == colors::Red);
        REQUIRE(color::FromString("Blue") == colors::Blue);
        REQUIRE(color::FromString("RebeccaPurple") == colors::RebeccaPurple);

        REQUIRE(color::FromString("#FF00FF") == color {0XFF, 0X00, 0XFF, 0XFF});
        REQUIRE(color::FromString("#ABCD01") == color {0xAB, 0XCD, 0X01, 255});
        REQUIRE(color::FromString("#FFAABB14") == color {0XFF, 0XAA, 0XBB, 0X14});
        REQUIRE(color::FromString("#01020304") == color {1, 2, 3, 4});
    }

    SUBCASE("Literals")
    {
        using namespace tcob::literals;
        REQUIRE(color::FromString("Red") == "Red"_color);
        REQUIRE(color::FromString("Blue") == "Blue"_color);
        REQUIRE(color::FromString("RebeccaPurple") == "RebeccaPurple"_color);
    }

    SUBCASE("FromHSLA")
    {
        REQUIRE(color::FromHSLA(0, 0, 0) == colors::Black);
        REQUIRE(color::FromHSLA(240, 1.f, 0.5f) == colors::Blue);
        REQUIRE(color::FromHSLA(0, 1.f, 0.5f) == colors::Red);
        REQUIRE(color::FromHSLA(43, 0.744f, 0.49f) == color {217, 165, 31});
    }

    SUBCASE("Format")
    {
        REQUIRE(std::format("{}", colors::SandyBrown) == "(r:244,g:164,b:96,a:255)");
    }
}

TEST_CASE("GFX.ColorGradient.SingleColor")
{
    gfx::color_gradient grad {colors::Green, colors::Green};
    auto const&         colors {grad.get_colors()};
    for (usize i {0}; i < colors.size(); ++i) {
        REQUIRE(colors[i] == colors::Green);
    }
}

TEST_CASE("GFX.ColorGradient.TwoColors")
{
    SUBCASE("grayscale")
    {
        color_gradient grad {colors::Black, colors::White};
        auto const&    colors {grad.get_color_array(1)};
        for (usize i {0}; i < colors.size(); ++i) {
            f32 val {i / 255.f};
            REQUIRE(colors[i] == std::array {val, val, val, 1.f});
        }

        color_gradient grad2 {colors::Black, colors::White};
        REQUIRE(grad == grad2);
    }
    SUBCASE("red to blue")
    {
        color_gradient grad {colors::Red, colors::Blue};
        auto const&    colors {grad.get_color_array(1)};
        REQUIRE(colors[0] == std::array {1.f, 0.f, 0.f, 1.f});
        f32 val {127 / 255.f};
        REQUIRE(colors[127] == std::array {val, 0.f, val, 1.f});
        REQUIRE(colors[255] == std::array {0.f, 0.f, 1.f, 1.f});
    }
}
