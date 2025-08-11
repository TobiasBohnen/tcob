// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/ui/Paint.hpp"

#include <variant>

#include "tcob/core/AngleUnits.hpp"
#include "tcob/core/Color.hpp"
#include "tcob/core/Size.hpp"
#include "tcob/gfx/ColorGradient.hpp"
#include "tcob/gfx/ui/UI.hpp"

namespace tcob::ui {

////////////////////////////////////////////////////////////

void paint_lerp(paint& target, paint const& left, paint const& right, f64 step)
{
    if (auto const* lc {std::get_if<color>(&left)}) {
        if (auto const* rc {std::get_if<color>(&right)}) {
            target = color::Lerp(*lc, *rc, step);
        }
        return;
    }
    if (auto const* lc {std::get_if<linear_gradient>(&left)}) {
        if (auto const* rc {std::get_if<linear_gradient>(&right)}) {
            linear_gradient grad;
            grad.Angle  = degree_f::Lerp(lc->Angle, rc->Angle, step);
            grad.Colors = gfx::color_gradient::Lerp(lc->Colors, rc->Colors, step);
            target      = grad;
        }
        return;
    }
    if (auto const* lc {std::get_if<radial_gradient>(&left)}) {
        if (auto const* rc {std::get_if<radial_gradient>(&right)}) {
            radial_gradient grad;
            grad.InnerRadius = length::Lerp(lc->InnerRadius, rc->InnerRadius, step);
            grad.OuterRadius = length::Lerp(lc->OuterRadius, rc->OuterRadius, step);
            grad.Scale       = size_f::Lerp(lc->Scale, rc->Scale, step);
            grad.Colors      = gfx::color_gradient::Lerp(lc->Colors, rc->Colors, step);
            target           = grad;
        }
        return;
    }
    if (auto const* lc {std::get_if<box_gradient>(&left)}) {
        if (auto const* rc {std::get_if<box_gradient>(&right)}) {
            box_gradient grad;
            grad.Radius  = length::Lerp(lc->Radius, rc->Radius, step);
            grad.Feather = length::Lerp(lc->Feather, rc->Feather, step);
            grad.Colors  = gfx::color_gradient::Lerp(lc->Colors, rc->Colors, step);
            target       = grad;
        }
        return;
    }
}

} // namespace ui
