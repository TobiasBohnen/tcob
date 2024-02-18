#include "tests.hpp"

TEST_CASE("Core.POD.Point")
{
    static_assert(std::is_copy_constructible_v<point_f>);
    static_assert(std::is_copy_assignable_v<point_f>);
    static_assert(std::is_move_constructible_v<point_f>);
    static_assert(std::is_move_assignable_v<point_f>);
    static_assert(POD<point_f>);

    SUBCASE("Construction")
    {
        // default constructor
        {
            point_f p;
            REQUIRE(p.X == 0.f);
            REQUIRE(p.Y == 0.f);
        }
        {
            point_i p;
            REQUIRE(p.X == 0);
            REQUIRE(p.Y == 0);
        }
        {
            point_u p;
            REQUIRE(p.X == 0);
            REQUIRE(p.Y == 0);
        }
        // x,y constructor
        {
            f32     x {2.45f};
            f32     y {4.21f};
            point_f p {x, y};
            REQUIRE(p.X == x);
            REQUIRE(p.Y == y);
        }
        {
            i32     x {2};
            i32     y {4};
            point_i p {x, y};
            REQUIRE(p.X == x);
            REQUIRE(p.Y == y);
        }
        {
            u32     x {2};
            u32     y {4};
            point_u p {x, y};
            REQUIRE(p.X == x);
            REQUIRE(p.Y == y);
        }
        // copy constructor
        {
            point_u p1 {2, 4};
            point_u p2 {p1};
            REQUIRE(p1 == p2);
        }
        {
            point_i p1 {2, 4};
            point_u p2 {p1};
            REQUIRE(p1.X == p2.X);
            REQUIRE(p1.Y == p2.Y);
        }
    }

    SUBCASE("Equality")
    {
        {
            point_f p1;
            point_f p2;
            REQUIRE(p1 == p2);
        }
        {
            point_i p1;
            point_i p2;
            REQUIRE(p1 == p2);
        }
        {
            point_u p1;
            point_u p2;
            REQUIRE(p1 == p2);
        }

        {
            f32     x {2.45f};
            f32     y {4.21f};
            point_f p1 {x, y};
            point_f p2 {x, y};
            REQUIRE(p1 == p2);
        }
        {
            i32     x {2};
            i32     y {4};
            point_i p1 {x, y};
            point_i p2 {x, y};
            REQUIRE(p1 == p2);
        }
        {
            u32     x {2};
            u32     y {4};
            point_u p1 {x, y};
            point_u p2 {x, y};
            REQUIRE(p1 == p2);
        }

        {
            point_f p1 {1, 2};
            point_f p2 {3, 4};
            REQUIRE(p1 != p2);
        }
        {
            point_f p1 {1, 2};
            point_f p2 {1, 4};
            REQUIRE(p1 != p2);
        }
        {
            point_f p1 {1, 2};
            point_f p2 {3, 2};
            REQUIRE(p1 != p2);
        }
        {
            point_i p1 {1, 2};
            point_i p2 {3, 4};
            REQUIRE(p1 != p2);
        }
        {
            point_u p1 {1, 2};
            point_u p2 {3, 4};
            REQUIRE(p1 != p2);
        }
        {
            f32 const x0 {2.45f};
            f32 const tol {0.1f};
            f32 const x1 {x0 + tol};
            f32 const x2 {x0 - tol};
            f32 const x3 {x0 + tol + std::numeric_limits<f32>::epsilon()};
            f32 const x4 {x0 - tol - std::numeric_limits<f32>::epsilon()};
            REQUIRE(point_f {x0, 0}.equals(point_f {x1, 0}, tol));
            REQUIRE(point_f {x0, 0}.equals(point_f {x2, 0}, tol));
            REQUIRE_FALSE(point_f {x0, 0}.equals(point_f {x3, 0}, tol));
            REQUIRE_FALSE(point_f {x0, 0}.equals(point_f {x4, 0}, tol));
        }
    }

    SUBCASE("Addition")
    {
        {
            point_f p1 {2.45f, 4.21f};
            point_f p2 {1.39f, 61.21f};

            point_f p3 {p1 + p2};
            REQUIRE(p3.X == (p1.X + p2.X));
            REQUIRE(p3.Y == (p1.Y + p2.Y));

            p1 += p2;
            REQUIRE(p3 == p1);
        }
        {
            point_i p1 {3, 5};
            point_i p2 {4, 6};

            point_i p3 {p1 + p2};
            REQUIRE(p3.X == (p1.X + p2.X));
            REQUIRE(p3.Y == (p1.Y + p2.Y));

            p1 += p2;
            REQUIRE(p3 == p1);
        }
        {
            point_u p1 {3, 5};
            point_u p2 {4, 6};

            point_u p3 {p1 + p2};
            REQUIRE(p3.X == (p1.X + p2.X));
            REQUIRE(p3.Y == (p1.Y + p2.Y));

            p1 += p2;
            REQUIRE(p3 == p1);
        }
    }

    SUBCASE("Subtraction")
    {
        {
            point_f p1 {2.45f, 4.21f};
            point_f p2 {1.39f, 61.21f};

            point_f p3 {p1 - p2};
            REQUIRE(p3.X == (p1.X - p2.X));
            REQUIRE(p3.Y == (p1.Y - p2.Y));

            p1 -= p2;
            REQUIRE(p3 == p1);
        }
        {
            point_i p1 {3, 5};
            point_i p2 {1, 2};

            point_i p3 {p1 - p2};
            REQUIRE(p3.X == (p1.X - p2.X));
            REQUIRE(p3.Y == (p1.Y - p2.Y));

            p1 -= p2;
            REQUIRE(p3 == p1);
        }
        {
            point_u p1 {3, 5};
            point_u p2 {1, 2};

            point_u p3 {p1 - p2};
            REQUIRE(p3.X == (p1.X - p2.X));
            REQUIRE(p3.Y == (p1.Y - p2.Y));

            p1 -= p2;
            REQUIRE(p3 == p1);
        }
    }

    SUBCASE("Multiplication")
    {
        {
            point_f p1 {2.45f, 4.21f};
            point_f p2 {1.39f, 61.21f};

            point_f p3 {p1 * p2};
            REQUIRE(p3.X == (p1.X * p2.X));
            REQUIRE(p3.Y == (p1.Y * p2.Y));

            p1 *= p2;
            REQUIRE(p3 == p1);
        }
        {
            point_f p1 {2.45f, 4.21f};
            f32     p2 {10.5f};

            point_f p3 {p1 * p2};
            REQUIRE(p3.X == (p2 * p1.X));
            REQUIRE(p3.Y == (p2 * p1.Y));

            p1 *= p2;
            REQUIRE(p3 == p1);
        }
        {
            point_i p1 {3, 5};
            point_i p2 {1, 2};

            point_i p3 {p1 * p2};
            REQUIRE(p3.X == (p1.X * p2.X));
            REQUIRE(p3.Y == (p1.Y * p2.Y));

            p1 *= p2;
            REQUIRE(p3 == p1);
        }
        {
            point_i p1 {2, 4};
            i32     p2 {-10};

            point_i p3 {p1 * p2};
            REQUIRE(p3.X == (p2 * p1.X));
            REQUIRE(p3.Y == (p2 * p1.Y));

            p1 *= p2;
            REQUIRE(p3 == p1);
        }
        {
            point_u p1 {3, 5};
            point_u p2 {1, 2};

            point_u p3 {p1 * p2};
            REQUIRE(p3.X == (p1.X * p2.X));
            REQUIRE(p3.Y == (p1.Y * p2.Y));

            p1 *= p2;
            REQUIRE(p3 == p1);
        }
        {
            point_u p1 {2, 4};
            u32     p2 {10};

            point_u p3 {p1 * p2};
            REQUIRE(p3.X == (p2 * p1.X));
            REQUIRE(p3.Y == (p2 * p1.Y));

            p1 *= p2;
            REQUIRE(p3 == p1);
        }
    }

    SUBCASE("Division")
    {
        {
            point_f p1 {2.45f, 4.21f};
            point_f p2 {1.39f, 61.21f};

            point_f p3 {p1 / p2};
            REQUIRE(p3.X == (p1.X / p2.X));
            REQUIRE(p3.Y == (p1.Y / p2.Y));

            p1 /= p2;
            REQUIRE(p3 == p1);
        }
        {
            point_f p1 {2.45f, 4.21f};
            f32     p2 {10.5f};

            point_f p3 {p1 / p2};
            REQUIRE(p3.X == (p1.X / p2));
            REQUIRE(p3.Y == (p1.Y / p2));

            p1 /= p2;
            REQUIRE(p3 == p1);
        }
        {
            point_i p1 {30, 50};
            point_i p2 {3, 5};

            point_i p3 {p1 / p2};
            REQUIRE(p3.X == (p1.X / p2.X));
            REQUIRE(p3.Y == (p1.Y / p2.Y));

            p1 /= p2;
            REQUIRE(p3 == p1);
        }
        {
            point_i p1 {20, 40};
            i32     p2 {-10};

            point_i p3 {p1 / p2};
            REQUIRE(p3.X == (p1.X / p2));
            REQUIRE(p3.Y == (p1.Y / p2));

            p1 /= p2;
            REQUIRE(p3 == p1);
        }
        {
            point_u p1 {30, 50};
            point_u p2 {3, 5};

            point_u p3 {p1 / p2};
            REQUIRE(p3.X == (p1.X / p2.X));
            REQUIRE(p3.Y == (p1.Y / p2.Y));

            p1 /= p2;
            REQUIRE(p3 == p1);
        }
        {
            point_u p1 {20, 40};
            u32     p2 {10};

            point_u p3 {p1 / p2};
            REQUIRE(p3.X == (p1.X / p2));
            REQUIRE(p3.Y == (p1.Y / p2));

            p1 /= p2;
            REQUIRE(p3 == p1);
        }
    }
    SUBCASE("length")
    {
        REQUIRE(point_f::Zero.length() == 0.f);
        REQUIRE(point_f {3, 4}.length() == 5.f);
        REQUIRE(point_f {8, 15}.length() == 17.f);
        REQUIRE(point_i {8, 15}.length() == 17.f);
    }
    SUBCASE("distance_to")
    {
        REQUIRE(point_f::Zero.distance_to(point_f::Zero) == 0.f);
        REQUIRE(point_f {8, 11}.distance_to(point_f {5, 7}) == 5.f);
    }
    SUBCASE("distance_to")
    {
        REQUIRE(point_f::Zero.as_normalized() == point_f::Zero);
        REQUIRE(point_f {3, 4}.as_normalized() == point_f {0.6f, 0.8f});
        REQUIRE(point_f {7, 24}.as_normalized() == point_f {0.28f, 0.96f});
    }
    SUBCASE("Format")
    {
        REQUIRE(std::format("{}", point_f {10.123f, 23.434f}) == "(x:10.123,y:23.434)");
    }
}
