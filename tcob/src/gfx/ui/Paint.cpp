// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/ui/Paint.hpp"

#include <cmath>
#include <variant>

#include "tcob/core/AngleUnits.hpp"
#include "tcob/core/Color.hpp"
#include "tcob/core/Common.hpp"
#include "tcob/core/Size.hpp"
#include "tcob/gfx/ColorGradient.hpp"
#include "tcob/gfx/ui/UI.hpp"

namespace tcob::ui {

////////////////////////////////////////////////////////////

void paint_lerp(paint& target, paint const& from, paint const& to, f64 step)
{
    if (auto const* lc {std::get_if<color>(&from)}) {
        if (auto const* rc {std::get_if<color>(&to)}) {
            target = helper::lerp(*lc, *rc, step);
        } else {
            target = to;
        }
        return;
    }

    // quantize step for color gradients
    constexpr f64 increment {0.05};
    f64 const     qstep {std::round(step / increment) * increment};

    if (auto const* lc {std::get_if<linear_gradient>(&from)}) {
        if (auto const* rc {std::get_if<linear_gradient>(&to)}) {
            linear_gradient grad;
            grad.Angle  = helper::lerp(lc->Angle, rc->Angle, step);
            grad.Colors = helper::lerp(lc->Colors, rc->Colors, qstep);
            target      = grad;
        } else {
            target = to;
        }
        return;
    }
    if (auto const* lc {std::get_if<radial_gradient>(&from)}) {
        if (auto const* rc {std::get_if<radial_gradient>(&to)}) {
            radial_gradient grad;
            grad.InnerRadius = helper::lerp(lc->InnerRadius, rc->InnerRadius, step);
            grad.OuterRadius = helper::lerp(lc->OuterRadius, rc->OuterRadius, step);
            grad.Scale       = helper::lerp(lc->Scale, rc->Scale, step);
            grad.Colors      = helper::lerp(lc->Colors, rc->Colors, qstep);
            target           = grad;
        } else {
            target = to;
        }
        return;
    }
    if (auto const* lc {std::get_if<box_gradient>(&from)}) {
        if (auto const* rc {std::get_if<box_gradient>(&to)}) {
            box_gradient grad;
            grad.Radius  = helper::lerp(lc->Radius, rc->Radius, step);
            grad.Feather = helper::lerp(lc->Feather, rc->Feather, step);
            grad.Colors  = helper::lerp(lc->Colors, rc->Colors, qstep);
            target       = grad;
        } else {
            target = to;
        }
        return;
    }
    if (auto const* lc {std::get_if<conic_gradient>(&from)}) {
        if (auto const* rc {std::get_if<conic_gradient>(&to)}) {
            conic_gradient grad;
            grad.Colors = helper::lerp(lc->Colors, rc->Colors, qstep);
            target      = grad;
        } else {
            target = to;
        }
        return;
    }
}

}
