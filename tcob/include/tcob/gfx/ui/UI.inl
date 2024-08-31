// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "UI.hpp"

namespace tcob::gfx::ui {

inline auto operator/(length const& left, f32 right) -> length
{
    return {left.Value / right, left.Type};
}

inline auto operator-(rect_f const& left, thickness const& right) -> rect_f
{
    return {left.X + right.Left.calc(left.Width),
            left.Y + right.Top.calc(left.Width),
            left.Width - (right.Left.calc(left.Width) + right.Right.calc(left.Width)),
            left.Height - (right.Top.calc(left.Width) + right.Bottom.calc(left.Width))};
}

inline auto operator-=(rect_f& left, thickness const& right) -> rect_f&
{
    left.X += right.Left.calc(left.Width);
    left.Y += right.Top.calc(left.Width);
    left.Width -= right.Left.calc(left.Width) + right.Right.calc(left.Width);
    left.Height -= right.Top.calc(left.Width) + right.Bottom.calc(left.Width);

    return left;
}

}
