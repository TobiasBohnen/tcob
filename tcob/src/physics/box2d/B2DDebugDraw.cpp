// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/physics/box2d/B2DDebugDraw.hpp"

#if defined(TCOB_ENABLE_ADDON_PHYSICS_BOX2D)

    #include <box2d/b2_draw.h>

namespace tcob::physics::box2d {

auto static ConvertColor(b2Color col) -> color
{
    return {static_cast<u8>(col.r * 255), static_cast<u8>(col.g * 255), static_cast<u8>(col.b * 255), static_cast<u8>(col.a * 255)};
}

struct ddraw : public b2Draw {
    ddraw(debug_draw* parent)
        : _parent {parent}
    {
    }

    /// Draw a closed polygon provided in CCW order.
    void DrawPolygon(b2Vec2 const* vertices, int32 vertexCount, b2Color const& color) override
    {
        auto verts {std::span<point_f const> {reinterpret_cast<point_f const*>(vertices), static_cast<usize>(vertexCount)}};
        _parent->draw_polygon(verts, ConvertColor(color));
    }

    /// Draw a solid closed polygon provided in CCW order.
    void DrawSolidPolygon(b2Vec2 const* vertices, int32 vertexCount, b2Color const& color) override
    {
        auto verts {std::span<point_f const> {reinterpret_cast<point_f const*>(vertices), static_cast<usize>(vertexCount)}};
        _parent->draw_solid_polygon(verts, ConvertColor(color));
    }

    /// Draw a circle.
    void DrawCircle(b2Vec2 const& center, float radius, b2Color const& color) override
    {
        _parent->draw_circle({center.x, center.y}, radius, ConvertColor(color));
    }

    /// Draw a solid circle.
    void DrawSolidCircle(b2Vec2 const& center, float radius, b2Vec2 const& axis, b2Color const& color) override
    {
        _parent->draw_solid_circle({center.x, center.y}, radius, {axis.x, axis.y}, ConvertColor(color));
    }

    /// Draw a line segment.
    void DrawSegment(b2Vec2 const& p1, b2Vec2 const& p2, b2Color const& color) override
    {
        _parent->draw_segment({p1.x, p1.y}, {p2.x, p2.y}, ConvertColor(color));
    }

    /// Draw a transform. Choose your own length scale.
    /// @param xf a transform.
    void DrawTransform(b2Transform const& xf) override
    {
        _parent->draw_transform({.Position = {xf.p.x, xf.p.y}, .Angle = xf.q.GetAngle()});
    }

    /// Draw a point.
    void DrawPoint(b2Vec2 const& p, float size, b2Color const& color) override
    {
        _parent->draw_point({p.x, p.y}, size, ConvertColor(color));
    }

    debug_draw* _parent {};
};

debug_draw::debug_draw()
    : _b2Draw {new ddraw(this)}
{
    _b2Draw->SetFlags(b2Draw::e_shapeBit | b2Draw::e_aabbBit | b2Draw::e_centerOfMassBit | b2Draw::e_jointBit | b2Draw::e_pairBit);
}

debug_draw::~debug_draw()
{
    delete _b2Draw;
}

}

#endif
