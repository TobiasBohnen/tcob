// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "UI.hpp"

#include "tcob/core/Rect.hpp"

namespace tcob::ui {

inline auto operator/(length const& left, f32 right) -> length
{
    return {left.Value / right, left.Type};
}

inline auto operator-(rect_f const& left, thickness const& right) -> rect_f
{
    f32 const l {right.Left.calc(left.width())};
    f32 const r {right.Right.calc(left.width())};
    f32 const t {right.Top.calc(left.width())};
    f32 const b {right.Bottom.calc(left.width())};

    return {left.left() + l,
            left.top() + t,
            left.width() - (l + r),
            left.height() - (t + b)};
}

inline auto operator-=(rect_f& left, thickness const& right) -> rect_f&
{
    f32 const l {right.Left.calc(left.width())};
    f32 const r {right.Right.calc(left.width())};
    f32 const t {right.Top.calc(left.width())};
    f32 const b {right.Bottom.calc(left.width())};

    left.Position.X += l;
    left.Position.Y += t;
    left.Size.Width -= (l + r);
    left.Size.Height -= (t + b);

    return left;
}

inline auto operator+(rect_f const& left, thickness const& right) -> rect_f
{
    f32 const l {right.Left.calc(left.width())};
    f32 const r {right.Right.calc(left.width())};
    f32 const t {right.Top.calc(left.width())};
    f32 const b {right.Bottom.calc(left.width())};

    return {left.left() - l,
            left.top() - t,
            left.width() + (l + r),
            left.height() + (t + b)};
}

inline auto operator+=(rect_f& left, thickness const& right) -> rect_f&
{
    f32 const l {right.Left.calc(left.width())};
    f32 const r {right.Right.calc(left.width())};
    f32 const t {right.Top.calc(left.width())};
    f32 const b {right.Bottom.calc(left.width())};

    left.Position.X -= l;
    left.Position.Y -= t;
    left.Size.Width += (l + r);
    left.Size.Height += (t + b);

    return left;
}

}
