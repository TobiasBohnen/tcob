#include "tests.hpp"

TEST_CASE("Core.Data.Point")
{
    static_assert(std::is_copy_constructible_v<PointF>);
    static_assert(std::is_copy_assignable_v<PointF>);
    static_assert(std::is_move_constructible_v<PointF>);
    static_assert(std::is_move_assignable_v<PointF>);

    SECTION("Construction")
    {
        // default constructor
        {
            PointF p;
            REQUIRE(p.X == 0.f);
            REQUIRE(p.Y == 0.f);
        }
        {
            PointI p;
            REQUIRE(p.X == 0);
            REQUIRE(p.Y == 0);
        }
        {
            PointU p;
            REQUIRE(p.X == 0);
            REQUIRE(p.Y == 0);
        }
        // x,y constructor
        {
            f32 x { 2.45f };
            f32 y { 4.21f };
            PointF p { x, y };
            REQUIRE(p.X == x);
            REQUIRE(p.Y == y);
        }
        {
            i32 x { 2 };
            i32 y { 4 };
            PointI p { x, y };
            REQUIRE(p.X == x);
            REQUIRE(p.Y == y);
        }
        {
            u32 x { 2 };
            u32 y { 4 };
            PointU p { x, y };
            REQUIRE(p.X == x);
            REQUIRE(p.Y == y);
        }
        // copy constructor
        {
            PointU p1 { 2, 4 };
            PointU p2 { p1 };
            REQUIRE(p1 == p2);
        }
        {
            PointI p1 { 2, 4 };
            PointU p2 { p1 };
            REQUIRE(p1.X == p2.X);
            REQUIRE(p1.Y == p2.Y);
        }
    }

    SECTION("Equality")
    {
        {
            PointF p1;
            PointF p2;
            REQUIRE(p1 == p2);
        }
        {
            PointI p1;
            PointI p2;
            REQUIRE(p1 == p2);
        }
        {
            PointU p1;
            PointU p2;
            REQUIRE(p1 == p2);
        }

        {
            f32 x { 2.45f };
            f32 y { 4.21f };
            PointF p1 { x, y };
            PointF p2 { x, y };
            REQUIRE(p1 == p2);
        }
        {
            i32 x { 2 };
            i32 y { 4 };
            PointI p1 { x, y };
            PointI p2 { x, y };
            REQUIRE(p1 == p2);
        }
        {
            u32 x { 2 };
            u32 y { 4 };
            PointU p1 { x, y };
            PointU p2 { x, y };
            REQUIRE(p1 == p2);
        }

        {
            PointF p1 { 1, 2 };
            PointF p2 { 3, 4 };
            REQUIRE(p1 != p2);
        }
        {
            PointF p1 { 1, 2 };
            PointF p2 { 1, 4 };
            REQUIRE(p1 != p2);
        }
        {
            PointF p1 { 1, 2 };
            PointF p2 { 3, 2 };
            REQUIRE(p1 != p2);
        }
        {
            PointI p1 { 1, 2 };
            PointI p2 { 3, 4 };
            REQUIRE(p1 != p2);
        }
        {
            PointU p1 { 1, 2 };
            PointU p2 { 3, 4 };
            REQUIRE(p1 != p2);
        }
    }

    SECTION("Addition")
    {
        {
            PointF p1 { 2.45f, 4.21f };
            PointF p2 { 1.39f, 61.21f };

            PointF p3 { p1 + p2 };
            REQUIRE(p3.X == (p1.X + p2.X));
            REQUIRE(p3.Y == (p1.Y + p2.Y));

            p1 += p2;
            REQUIRE(p3 == p1);
        }
        {
            PointI p1 { 3, 5 };
            PointI p2 { 4, 6 };

            PointI p3 { p1 + p2 };
            REQUIRE(p3.X == (p1.X + p2.X));
            REQUIRE(p3.Y == (p1.Y + p2.Y));

            p1 += p2;
            REQUIRE(p3 == p1);
        }
        {
            PointU p1 { 3, 5 };
            PointU p2 { 4, 6 };

            PointU p3 { p1 + p2 };
            REQUIRE(p3.X == (p1.X + p2.X));
            REQUIRE(p3.Y == (p1.Y + p2.Y));

            p1 += p2;
            REQUIRE(p3 == p1);
        }
    }

    SECTION("Subtraction")
    {
        {
            PointF p1 { 2.45f, 4.21f };
            PointF p2 { 1.39f, 61.21f };

            PointF p3 { p1 - p2 };
            REQUIRE(p3.X == (p1.X - p2.X));
            REQUIRE(p3.Y == (p1.Y - p2.Y));

            p1 -= p2;
            REQUIRE(p3 == p1);
        }
        {
            PointI p1 { 3, 5 };
            PointI p2 { 1, 2 };

            PointI p3 { p1 - p2 };
            REQUIRE(p3.X == (p1.X - p2.X));
            REQUIRE(p3.Y == (p1.Y - p2.Y));

            p1 -= p2;
            REQUIRE(p3 == p1);
        }
        {
            PointU p1 { 3, 5 };
            PointU p2 { 1, 2 };

            PointU p3 { p1 - p2 };
            REQUIRE(p3.X == (p1.X - p2.X));
            REQUIRE(p3.Y == (p1.Y - p2.Y));

            p1 -= p2;
            REQUIRE(p3 == p1);
        }
    }

    SECTION("Multiplication")
    {
        {
            PointF p1 { 2.45f, 4.21f };
            PointF p2 { 1.39f, 61.21f };

            PointF p3 { p1 * p2 };
            REQUIRE(p3.X == (p1.X * p2.X));
            REQUIRE(p3.Y == (p1.Y * p2.Y));

            p1 *= p2;
            REQUIRE(p3 == p1);
        }
        {
            PointF p1 { 2.45f, 4.21f };
            f32 p2 { 10.5f };

            PointF p3 { p1 * p2 };
            REQUIRE(p3.X == (p2 * p1.X));
            REQUIRE(p3.Y == (p2 * p1.Y));

            p1 *= p2;
            REQUIRE(p3 == p1);
        }
        {
            PointI p1 { 3, 5 };
            PointI p2 { 1, 2 };

            PointI p3 { p1 * p2 };
            REQUIRE(p3.X == (p1.X * p2.X));
            REQUIRE(p3.Y == (p1.Y * p2.Y));

            p1 *= p2;
            REQUIRE(p3 == p1);
        }
        {
            PointI p1 { 2, 4 };
            i32 p2 { -10 };

            PointI p3 { p1 * p2 };
            REQUIRE(p3.X == (p2 * p1.X));
            REQUIRE(p3.Y == (p2 * p1.Y));

            p1 *= p2;
            REQUIRE(p3 == p1);
        }
        {
            PointU p1 { 3, 5 };
            PointU p2 { 1, 2 };

            PointU p3 { p1 * p2 };
            REQUIRE(p3.X == (p1.X * p2.X));
            REQUIRE(p3.Y == (p1.Y * p2.Y));

            p1 *= p2;
            REQUIRE(p3 == p1);
        }
        {
            PointU p1 { 2, 4 };
            u32 p2 { 10 };

            PointU p3 { p1 * p2 };
            REQUIRE(p3.X == (p2 * p1.X));
            REQUIRE(p3.Y == (p2 * p1.Y));

            p1 *= p2;
            REQUIRE(p3 == p1);
        }
    }

    SECTION("Division")
    {
        {
            PointF p1 { 2.45f, 4.21f };
            PointF p2 { 1.39f, 61.21f };

            PointF p3 { p1 / p2 };
            REQUIRE(p3.X == (p1.X / p2.X));
            REQUIRE(p3.Y == (p1.Y / p2.Y));

            p1 /= p2;
            REQUIRE(p3 == p1);
        }
        {
            PointF p1 { 2.45f, 4.21f };
            f32 p2 { 10.5f };

            PointF p3 { p1 / p2 };
            REQUIRE(p3.X == (p1.X / p2));
            REQUIRE(p3.Y == (p1.Y / p2));

            p1 /= p2;
            REQUIRE(p3 == p1);
        }
        {
            PointI p1 { 30, 50 };
            PointI p2 { 3, 5 };

            PointI p3 { p1 / p2 };
            REQUIRE(p3.X == (p1.X / p2.X));
            REQUIRE(p3.Y == (p1.Y / p2.Y));

            p1 /= p2;
            REQUIRE(p3 == p1);
        }
        {
            PointI p1 { 20, 40 };
            i32 p2 { -10 };

            PointI p3 { p1 / p2 };
            REQUIRE(p3.X == (p1.X / p2));
            REQUIRE(p3.Y == (p1.Y / p2));

            p1 /= p2;
            REQUIRE(p3 == p1);
        }
        {
            PointU p1 { 30, 50 };
            PointU p2 { 3, 5 };

            PointU p3 { p1 / p2 };
            REQUIRE(p3.X == (p1.X / p2.X));
            REQUIRE(p3.Y == (p1.Y / p2.Y));

            p1 /= p2;
            REQUIRE(p3 == p1);
        }
        {
            PointU p1 { 20, 40 };
            u32 p2 { 10 };

            PointU p3 { p1 / p2 };
            REQUIRE(p3.X == (p1.X / p2));
            REQUIRE(p3.Y == (p1.Y / p2));

            p1 /= p2;
            REQUIRE(p3 == p1);
        }
    }
}