// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/ui/Style.hpp"

#include "tcob/core/easing/Easing.hpp"
#include "tcob/gfx/ui/StyleElements.hpp"
#include "tcob/gfx/ui/UI.hpp"

namespace tcob::ui {
////////////////////////////////////////////////////////////

auto style::ease_value(f64 t) const -> f64
{
    switch (EasingFunc) {
    case easing_func::Linear:       return t;
    case easing_func::SmoothStep:   return easing::smoothstep<f64> {.Start = 0, .End = 1.}(t); break;
    case easing_func::SmootherStep: return easing::smootherstep<f64> {.Start = 0, .End = 1.}(t); break;
    case easing_func::QuadIn:       return easing::power<f64> {.Start = 0, .End = 1., .Exponent = 2.}(t); break;
    case easing_func::QuadOut:      return easing::inverse_power<f64> {.Start = 0, .End = 1., .Exponent = 2.}(t); break;
    case easing_func::QuadInOut:    return easing::inout_power<f64> {.Start = 0, .End = 1., .Exponent = 2.}(t); break;
    case easing_func::CubicIn:      return easing::power<f64> {.Start = 0, .End = 1, .Exponent = 3.}(t); break;
    case easing_func::CubicOut:     return easing::inverse_power<f64> {.Start = 0, .End = 1, .Exponent = 3.}(t); break;
    case easing_func::CubicInOut:   return easing::inout_power<f64> {.Start = 0, .End = 1., .Exponent = 3.}(t); break;
    case easing_func::QuartIn:      return easing::power<f64> {.Start = 0, .End = 1, .Exponent = 4.}(t); break;
    case easing_func::QuartOut:     return easing::inverse_power<f64> {.Start = 0, .End = 1, .Exponent = 4.}(t); break;
    case easing_func::QuartInOut:   return easing::inout_power<f64> {.Start = 0, .End = 1., .Exponent = 4.}(t); break;
    case easing_func::QuintIn:      return easing::power<f64> {.Start = 0, .End = 1, .Exponent = 5.}(t); break;
    case easing_func::QuintOut:     return easing::inverse_power<f64> {.Start = 0, .End = 1, .Exponent = 5.}(t); break;
    case easing_func::QuintInOut:   return easing::inout_power<f64> {.Start = 0, .End = 1., .Exponent = .5}(t); break;
    case easing_func::ExpoIn:       return easing::exponential<f64> {.Start = 0, .End = 1.}(t); break;
    case easing_func::ExpoOut:      return easing::inverse_exponential<f64> {.Start = 0, .End = 1.}(t); break;
    case easing_func::ExpoInOut:    return easing::inout_exponential<f64> {.Start = 0, .End = 1.}(t); break;
    }

    return t;
}

////////////////////////////////////////////////////////////

void widget_style::Transition(widget_style& target, widget_style const& left, widget_style const& right, f64 step)
{
    target.Padding = thickness::Lerp(left.Padding, right.Padding, step);
    target.Margin  = thickness::Lerp(left.Margin, right.Margin, step);
    paint_lerp(target.Background, left.Background, right.Background, step);

    target.DropShadow.lerp(left.DropShadow, right.DropShadow, step);
    target.Border.lerp(left.Border, right.Border, step);
}

////////////////////////////////////////////////////////////

void thumb_style::Transition(thumb_style& target, thumb_style const& left, thumb_style const& right, f64 step)
{
    target.Thumb.lerp(left.Thumb, right.Thumb, step);
}

void nav_arrows_style::Transition(nav_arrows_style& target, nav_arrows_style const& left, nav_arrows_style const& right, f64 step)
{
    target.NavArrow.lerp(left.NavArrow, right.NavArrow, step);
}

void item_style::Transition(item_style& target, item_style const& left, item_style const& right, f64 step)
{
    target.Item.lerp(left.Item, right.Item, step);
}

////////////////////////////////////////////////////////////

} // namespace ui
