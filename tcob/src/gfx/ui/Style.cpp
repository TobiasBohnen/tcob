// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/ui/Style.hpp"

#include "tcob/core/Common.hpp"
#include "tcob/core/tweening/TweenFunc.hpp"
#include "tcob/gfx/ui/Paint.hpp"
#include "tcob/gfx/ui/UI.hpp"
#include "tcob/gfx/ui/component/StyleElements.hpp"

namespace tcob::ui {
////////////////////////////////////////////////////////////

auto style::ease_value(f64 t) const -> f64
{
    using namespace tween_func;

    switch (EasingFunc) {
    case easing_func::Linear:        return t;
    case easing_func::SmoothStep:    return smoothstep<f64> {.Start = 0., .End = 1.}(t);
    case easing_func::SmootherStep:  return smootherstep<f64> {.Start = 0., .End = 1.}(t);
    case easing_func::QuadIn:        return polynomial<f64, ease_mode::In> {.Start = 0., .End = 1., .Exponent = 2.}(t);
    case easing_func::QuadOut:       return polynomial<f64, ease_mode::Out> {.Start = 0., .End = 1., .Exponent = 2.}(t);
    case easing_func::QuadInOut:     return polynomial<f64, ease_mode::InOut> {.Start = 0., .End = 1., .Exponent = 2.}(t);
    case easing_func::CubicIn:       return polynomial<f64, ease_mode::In> {.Start = 0., .End = 1., .Exponent = 3.}(t);
    case easing_func::CubicOut:      return polynomial<f64, ease_mode::Out> {.Start = 0, .End = 1., .Exponent = 3.}(t);
    case easing_func::CubicInOut:    return polynomial<f64, ease_mode::InOut> {.Start = 0., .End = 1., .Exponent = 3.}(t);
    case easing_func::QuartIn:       return polynomial<f64, ease_mode::In> {.Start = 0., .End = 1., .Exponent = 4.}(t);
    case easing_func::QuartOut:      return polynomial<f64, ease_mode::Out> {.Start = 0., .End = 1., .Exponent = 4.}(t);
    case easing_func::QuartInOut:    return polynomial<f64, ease_mode::InOut> {.Start = 0., .End = 1., .Exponent = 4.}(t);
    case easing_func::QuintIn:       return polynomial<f64, ease_mode::In> {.Start = 0., .End = 1., .Exponent = 5.}(t);
    case easing_func::QuintOut:      return polynomial<f64, ease_mode::Out> {.Start = 0., .End = 1., .Exponent = 5.}(t);
    case easing_func::QuintInOut:    return polynomial<f64, ease_mode::InOut> {.Start = 0., .End = 1., .Exponent = 5.}(t);
    case easing_func::ExpoIn:        return exponential<f64, ease_mode::In> {.Start = 0., .End = 1.}(t);
    case easing_func::ExpoOut:       return exponential<f64, ease_mode::Out> {.Start = 0., .End = 1.}(t);
    case easing_func::ExpoInOut:     return exponential<f64, ease_mode::InOut> {.Start = 0., .End = 1.}(t);
    case easing_func::BounceIn:      return bounce<f64, ease_mode::In> {.Start = 0., .End = 1.}(t);
    case easing_func::BounceOut:     return bounce<f64, ease_mode::Out> {.Start = 0., .End = 1.}(t);
    case easing_func::BounceInOut:   return bounce<f64, ease_mode::InOut> {.Start = 0., .End = 1.}(t);
    case easing_func::ElasticIn:     return elastic<f64, ease_mode::In> {.Start = 0., .End = 1.}(t);
    case easing_func::ElasticOut:    return elastic<f64, ease_mode::Out> {.Start = 0., .End = 1.}(t);
    case easing_func::ElasticInOut:  return elastic<f64, ease_mode::InOut> {.Start = 0., .End = 1.}(t);
    case easing_func::BackIn:        return back<f64, ease_mode::In> {.Start = 0., .End = 1.}(t);
    case easing_func::BackOut:       return back<f64, ease_mode::Out> {.Start = 0., .End = 1.}(t);
    case easing_func::BackInOut:     return back<f64, ease_mode::InOut> {.Start = 0., .End = 1.}(t);
    case easing_func::CircularIn:    return circular<f64, ease_mode::In> {.Start = 0., .End = 1.}(t);
    case easing_func::CircularOut:   return circular<f64, ease_mode::Out> {.Start = 0., .End = 1.}(t);
    case easing_func::CircularInOut: return circular<f64, ease_mode::InOut> {.Start = 0., .End = 1.}(t);
    }

    return t;
}

////////////////////////////////////////////////////////////

void widget_style::Transition(widget_style& target, widget_style const& from, widget_style const& to, f64 step)
{
    target.Padding = helper::lerp(from.Padding, to.Padding, step);
    target.Margin  = helper::lerp(from.Margin, to.Margin, step);
    paint_lerp(target.Background, from.Background, to.Background, step);

    target.DropShadow.lerp(from.DropShadow, to.DropShadow, step);
    target.Border.lerp(from.Border, to.Border, step);
}

////////////////////////////////////////////////////////////

void thumb_style::Transition(thumb_style& target, thumb_style const& from, thumb_style const& to, f64 step)
{
    target.Thumb.lerp(from.Thumb, to.Thumb, step);
}

void nav_arrows_style::Transition(nav_arrows_style& target, nav_arrows_style const& from, nav_arrows_style const& to, f64 step)
{
    target.NavArrow.lerp(from.NavArrow, to.NavArrow, step);
}

void item_style::Transition(item_style& target, item_style const& from, item_style const& to, f64 step)
{
    target.Item.lerp(from.Item, to.Item, step);
}

////////////////////////////////////////////////////////////

}
