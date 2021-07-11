// Copyright (c) 2021 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include <tcob/gfx/Quad.hpp>

namespace tcob {
static_assert(sizeof(Quad) == sizeof(Vertex) * 4);

void Quad::color(const Color& color)
{
    const std::array<u8, 4> col { color.R, color.G, color.B, color.A };
    TopRight.Color = col;
    BottomRight.Color = col;
    BottomLeft.Color = col;
    TopLeft.Color = col;
}

void Quad::texcoords(const TextureRegion& region)
{
    const RectF rect { region.UVRect };
    const f32 level { static_cast<f32>(region.Level) };

    TopRight.TexCoords = { rect.right(), rect.Top, level };
    BottomRight.TexCoords = { rect.right(), rect.bottom(), level };
    BottomLeft.TexCoords = { rect.Left, rect.bottom(), level };
    TopLeft.TexCoords = { rect.Left, rect.Top, level };
}

void Quad::scroll_texcoords(const PointF& offset)
{
    const f32 left { TopLeft.TexCoords[0] + offset.X };
    const f32 top { TopLeft.TexCoords[1] + offset.Y };
    const f32 right { BottomRight.TexCoords[0] + offset.X };
    const f32 bottom { BottomRight.TexCoords[1] + offset.Y };

    const RectF rect { left, top, right - left, bottom - top };
    const f32 level { TopRight.TexCoords[2] };

    TopRight.TexCoords = { rect.right(), rect.Top, level };
    BottomRight.TexCoords = { rect.right(), rect.bottom(), level };
    BottomLeft.TexCoords = { rect.Left, rect.bottom(), level };
    TopLeft.TexCoords = { rect.Left, rect.Top, level };
}

void Quad::position(const RectF& rect, const Transform& trans)
{
    const PointF tr { trans * rect.top_right() };
    TopRight.Position = { tr.X, tr.Y };

    const PointF br { trans * rect.bottom_right() };
    BottomRight.Position = { br.X, br.Y };

    const PointF bl { trans * rect.bottom_left() };
    BottomLeft.Position = { bl.X, bl.Y };

    const PointF tl { trans * rect.top_left() };
    TopLeft.Position = { tl.X, tl.Y };
}

void Quad::position(const RectF& rect)
{
    const PointF tr { rect.top_right() };
    TopRight.Position = { tr.X, tr.Y };

    const PointF br { rect.bottom_right() };
    BottomRight.Position = { br.X, br.Y };

    const PointF bl { rect.bottom_left() };
    BottomLeft.Position = { bl.X, bl.Y };

    const PointF tl { rect.top_left() };
    TopLeft.Position = { tl.X, tl.Y };
}
}