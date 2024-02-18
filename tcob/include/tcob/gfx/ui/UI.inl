// Copyright (c) 2023 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "UI.hpp"

namespace tcob::gfx::ui {

inline auto operator==(length const& left, length const& right) -> bool
{
    return left.Value == right.Value && right.Type == left.Type;
}

inline auto operator/(length const& left, f32 right) -> length
{
    return {left.Value / right, left.Type};
}

inline auto operator==(thickness const& left, thickness const& right) -> bool
{
    return left.Left == right.Left && right.Right == left.Right
        && left.Top == right.Top && right.Bottom == left.Bottom;
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

inline auto operator==(flags const& left, flags const& right) -> bool
{
    return left.as_array() == right.as_array();
}

inline auto operator==(linear_gradient const& left, linear_gradient const& right) -> bool
{
    return left.Angle == right.Angle && right.Colors == left.Colors;
}

inline auto operator==(radial_gradient const& left, radial_gradient const& right) -> bool
{
    return left.InnerRadius == right.InnerRadius && right.OuterRadius == left.OuterRadius
        && left.Scale == right.Scale && right.Colors == left.Colors;
}

inline auto operator==(box_gradient const& left, box_gradient const& right) -> bool
{
    return left.Radius == right.Radius && right.Feather == left.Feather
        && right.Colors == left.Colors;
}

inline auto operator==(image_pattern const& left, image_pattern const& right) -> bool
{
    return left.Stretch == right.Stretch && right.Texture == left.Texture;
}

inline auto operator==(nine_patch const& left, nine_patch const& right) -> bool
{
    return left.UV == right.UV && left.Region == right.Region && right.Texture == left.Texture;
}

}
