#include "tests.hpp"

TEST_CASE("Core.Data.Rect")
{
    static_assert(std::is_copy_constructible_v<RectF>);
    static_assert(std::is_copy_assignable_v<RectF>);
    static_assert(std::is_move_constructible_v<RectF>);
    static_assert(std::is_move_assignable_v<RectF>);

    SECTION("Construction")
    {
        // default constructor
        {
            RectF r;
            REQUIRE(r.Left == 0.f);
            REQUIRE(r.Top == 0.f);
            REQUIRE(r.Width == 0.f);
            REQUIRE(r.Height == 0.f);
        }
        {
            RectI r;
            REQUIRE(r.Left == 0);
            REQUIRE(r.Top == 0);
            REQUIRE(r.Width == 0);
            REQUIRE(r.Height == 0);
        }
        {
            RectU r;
            REQUIRE(r.Left == 0);
            REQUIRE(r.Top == 0);
            REQUIRE(r.Width == 0);
            REQUIRE(r.Height == 0);
        }
        // x,y,w,h constructor
        {
            f32 x { 2.45f };
            f32 y { 4.21f };
            f32 w { 12.15f };
            f32 h { 34.22f };
            RectF r { x, y, w, h };
            REQUIRE(r.Left == x);
            REQUIRE(r.Top == y);
            REQUIRE(r.Width == w);
            REQUIRE(r.Height == h);
        }
        {
            i32 x { 2 };
            i32 y { 4 };
            i32 w { 12 };
            i32 h { 45 };
            RectI r { x, y, w, h };
            REQUIRE(r.Left == x);
            REQUIRE(r.Top == y);
            REQUIRE(r.Width == w);
            REQUIRE(r.Height == h);
        }
        {
            u32 x { 2 };
            u32 y { 4 };
            u32 w { 12 };
            u32 h { 45 };
            RectU r { x, y, w, h };
            REQUIRE(r.Left == x);
            REQUIRE(r.Top == y);
            REQUIRE(r.Width == w);
            REQUIRE(r.Height == h);
        }
        // copy constructor
        {
            RectU p1 { 2, 4, 3, 4 };
            RectU p2 { p1 };
            REQUIRE(p1 == p2);
        }
        {
            RectI p1 { 2, 4, 3, 4 };
            RectU p2 { p1 };
            REQUIRE(p1.Left == p2.Left);
            REQUIRE(p1.Top == p2.Top);
            REQUIRE(p1.Width == p2.Width);
            REQUIRE(p1.Height == p2.Height);
        }
    }

    SECTION("Equality")
    {
        {
            RectF r1;
            RectF r2;
            REQUIRE(r1 == r2);
        }
        {
            RectI r1;
            RectI r2;
            REQUIRE(r1 == r2);
        }
        {
            RectU r1;
            RectU r2;
            REQUIRE(r1 == r2);
        }

        {
            f32 x { 2.45f };
            f32 y { 4.21f };
            f32 w { 12.45f };
            f32 h { 44.21f };
            RectF r1 { x, y, w, h };
            RectF r2 { x, y, w, h };
            REQUIRE(r1 == r2);
        }
        {
            i32 x { 2 };
            i32 y { 4 };
            i32 w { 12 };
            i32 h { 44 };
            RectI r1 { x, y, w, h };
            RectI r2 { x, y, w, h };
            REQUIRE(r1 == r2);
        }
        {
            u32 x { 2 };
            u32 y { 4 };
            u32 w { 12 };
            u32 h { 44 };
            RectU r1 { x, y, w, h };
            RectU r2 { x, y, w, h };
            REQUIRE(r1 == r2);
        }

        {
            RectU r1 { 0, 1, 2, 3 };
            RectU r2 { 0, 1, 2, 99 };
            REQUIRE(r1 != r2);
        }
        {
            RectU r1 { 0, 1, 2, 3 };
            RectU r2 { 0, 1, 99, 3 };
            REQUIRE(r1 != r2);
        }
        {
            RectU r1 { 0, 1, 2, 3 };
            RectU r2 { 0, 99, 2, 3 };
            REQUIRE(r1 != r2);
        }
        {
            RectU r1 { 0, 1, 2, 3 };
            RectU r2 { 99, 1, 2, 3 };
            REQUIRE(r1 != r2);
        }
    }

    SECTION("Contains")
    {
        {
            f32 x { 0.5f };
            f32 y { 2.5f };
            f32 w { 12.5f };
            f32 h { 3.5f };
            RectF r { x, y, w, h };
            REQUIRE(r.contains({ 0.75f, 3.9f }));
            REQUIRE_FALSE(r.contains({ 0.25f, 3.9f }));
            REQUIRE_FALSE(r.contains({ 0.75f, 7.9f }));
        }
        {
            i32 x { 2 };
            i32 y { 4 };
            i32 w { 12 };
            i32 h { 45 };
            RectI r { x, y, w, h };
            REQUIRE(r.contains({ 3, 5 }));
            REQUIRE_FALSE(r.contains({ 1, 17 }));
            REQUIRE_FALSE(r.contains({ 15, 5 }));
        }
    }

    SECTION("Intersects")
    {
        {
            RectF r1 { 0.5f, 1.5f, 2.5f, 3.5f };
            RectF r2 { 0.35f, 0.5f, 2.5f, 3.5f };
            REQUIRE(r1.intersects(r2));
            REQUIRE(r2.intersects(r1));
        }
        {
            RectF r1 { 0.5f, 1.5f, 2.5f, 3.5f };
            RectF r2 { 0.5f, 5.1f, 2.5f, 3.5f };
            REQUIRE_FALSE(r1.intersects(r2));
            REQUIRE_FALSE(r2.intersects(r1));
        }
    }

    SECTION("Center")
    {
        {
            RectI r1 { 5, 3, 10, 11 };
            REQUIRE(r1.center() == (PointF { 10, 8.5f }));
            REQUIRE(r1.center_local() == (PointF { 5.f, 5.5f }));
        }
        {
            RectF r1 { 5, 3, 10, 11 };
            REQUIRE(r1.center() == (PointF { 10, 8.5f }));
            REQUIRE(r1.center_local() == (PointF { 5.f, 5.5f }));
        }
    }
    SECTION("Structured Binding")
    {
        RectF r1 { 0.5f, 1.5f, 2.5f, 3.5f };
        auto [l, t, w, h] { r1 };
        REQUIRE(l == r1.Left);
        REQUIRE(t == r1.Top);
        REQUIRE(w == r1.Width);
        REQUIRE(h == r1.Height);
    }
    SECTION("Interpolate")
    {
        RectF r1 { 5, 8, 16, 25 };
        RectF r2 { 10, 16, 32, 50 };
        RectF r3 { r1.interpolate(r2, 0.5f) };
        REQUIRE(r3.Left == 7.5f);
        REQUIRE(r3.Top == 12.f);
        REQUIRE(r3.Width == 24.f);
        REQUIRE(r3.Height == 37.5f);
    }
    SECTION("FromLTRB")
    {
        RectI r1 { RectI::FromLTRB(10, 20, 30, 40) };
        REQUIRE(r1.Left == 10);
        REQUIRE(r1.right() == 30);
        REQUIRE(r1.Top == 20);
        REQUIRE(r1.bottom() == 40);

        REQUIRE(r1.Width == 20);
        REQUIRE(r1.Height == 20);
    }
}