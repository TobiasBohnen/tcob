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
    return {left.left() + right.Left.calc(left.width()),
            left.top() + right.Top.calc(left.width()),
            left.width() - (right.Left.calc(left.width()) + right.Right.calc(left.width())),
            left.height() - (right.Top.calc(left.width()) + right.Bottom.calc(left.width()))};
}

inline auto operator-=(rect_f& left, thickness const& right) -> rect_f&
{
    left.Position.X += right.Left.calc(left.width());
    left.Position.Y += right.Top.calc(left.width());
    left.Size.Width -= right.Left.calc(left.width()) + right.Right.calc(left.width());
    left.Size.Height -= right.Top.calc(left.width()) + right.Bottom.calc(left.width());

    return left;
}

}
