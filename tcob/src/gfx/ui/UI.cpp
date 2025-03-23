// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/ui/UI.hpp"

#include <algorithm>
#include <cassert>
#include <variant>

#include "tcob/core/AngleUnits.hpp"
#include "tcob/core/Color.hpp"
#include "tcob/core/Point.hpp"
#include "tcob/core/Size.hpp"
#include "tcob/core/input/Input.hpp"
#include "tcob/gfx/ColorGradient.hpp"
#include "tcob/gfx/ui/Form.hpp"
#include "tcob/gfx/ui/widgets/Widget.hpp"
#include "tcob/gfx/ui/widgets/WidgetContainer.hpp"

namespace tcob::ui {

////////////////////////////////////////////////////////////

length::length(f32 val, type t)
    : Value {val}
    , Type {t}
{
}

auto length::calc(f32 min, f32 refSize) const -> f32
{
    return std::max(min, calc(refSize));
}

auto length::calc(f32 refSize) const -> f32
{
    switch (Type) {
    case type::Relative: return Value * refSize;
    case type::Absolute: return Value;
    }

    return 0.0f;
}

auto length::operator-() const -> length
{
    return length {-Value, Type};
}

////////////////////////////////////////////////////////////

thickness::thickness(length l, length r, length t, length b)
    : Left {l}
    , Right {r}
    , Top {t}
    , Bottom {b}
{
}

thickness::thickness(length lr, length tb)
    : Left {lr}
    , Right {lr}
    , Top {tb}
    , Bottom {tb}
{
}

thickness::thickness(length all)
    : Left {all}
    , Right {all}
    , Top {all}
    , Bottom {all}
{
}

////////////////////////////////////////////////////////////

namespace detail {
    void input_injector::on_key_down(widget* widget, input::keyboard::event const& ev) const
    {
        if (check(widget)) { widget->do_key_down(ev); }
    }

    void input_injector::on_key_up(widget* widget, input::keyboard::event const& ev) const
    {
        if (check(widget)) { widget->do_key_up(ev); }
    }

    void input_injector::on_text_input(widget* widget, input::keyboard::text_input_event const& ev) const
    {
        if (check(widget)) { widget->do_text_input(ev); }
    }

    void input_injector::on_mouse_enter(widget* widget) const
    {
        if (check(widget)) { widget->do_mouse_enter(); }
    }

    void input_injector::on_mouse_leave(widget* widget) const
    {
        if (check(widget)) { widget->do_mouse_leave(); }
    }

    void input_injector::on_mouse_down(widget* widget, input::mouse::button_event const& ev) const
    {
        if (check(widget)) { widget->do_mouse_down(ev); }
    }

    void input_injector::on_mouse_up(widget* widget, input::mouse::button_event const& ev) const
    {
        if (check(widget)) { widget->do_mouse_up(ev); }
    }

    void input_injector::on_mouse_hover(widget* widget, input::mouse::motion_event const& ev) const
    {
        if (check(widget)) { widget->do_mouse_hover(ev); }
    }

    void input_injector::on_mouse_drag(widget* widget, input::mouse::motion_event const& ev) const
    {
        if (check(widget)) { widget->do_mouse_drag(ev); }
    }

    void input_injector::on_mouse_wheel(widget* widget, input::mouse::wheel_event const& ev) const
    {
        if (check(widget)) { widget->do_mouse_wheel(ev); }
    }

    void input_injector::on_controller_button_down(widget* widget, input::controller::button_event const& ev) const
    {
        if (check(widget)) { widget->do_controller_button_down(ev); }
    }

    void input_injector::on_controller_button_up(widget* widget, input::controller::button_event const& ev) const
    {
        if (check(widget)) { widget->do_controller_button_up(ev); }
    }

    void input_injector::on_click(widget* widget) const
    {
        if (check(widget)) { widget->do_click(); }
    }

    void input_injector::on_double_click(widget* widget) const
    {
        if (check(widget)) { widget->do_double_click(); }
    }

    void input_injector::on_focus_gained(widget* widget) const
    {
        if (check(widget)) { widget->do_focus_gained(); }
    }

    void input_injector::on_focus_lost(widget* widget) const
    {
        if (check(widget)) { widget->do_focus_lost(); }
    }

    auto input_injector::check(widget* widget) const -> bool
    {
        return widget && widget->is_enabled() && !widget->is_inert();
    }

}

////////////////////////////////////////////////////////////
auto length::Lerp(length const& left, length const& right, f64 step) -> length
{
    assert(left.Type == right.Type);
    length retValue;
    retValue.Type  = left.Type;
    retValue.Value = static_cast<f32>(left.Value + (right.Value - left.Value) * step);
    return retValue;
}

auto thickness::Lerp(thickness const& left, thickness const& right, f64 step) -> thickness
{
    thickness retValue;
    retValue.Bottom = length::Lerp(left.Bottom, right.Bottom, step);
    retValue.Top    = length::Lerp(left.Top, right.Top, step);
    retValue.Left   = length::Lerp(left.Left, right.Left, step);
    retValue.Right  = length::Lerp(left.Right, right.Right, step);
    return retValue;
}

auto dimensions::Lerp(dimensions const& left, dimensions const& right, f64 step) -> dimensions
{
    dimensions retValue;
    retValue.Width  = length::Lerp(left.Width, right.Width, step);
    retValue.Height = length::Lerp(left.Height, right.Height, step);
    return retValue;
}

auto global_to_content(widget const& widget, point_i p) -> point_f
{
    return point_f {p} - widget.global_content_bounds().Position;
}

auto global_to_parent(widget const& widget, point_i p) -> point_f
{
    point_f retValue {p};
    if (auto* parent {widget.parent()}) {
        retValue -= (parent->global_content_bounds().Position - parent->scroll_offset());
    } else {
        retValue -= point_f {widget.parent_form()->Bounds->Position};
    }

    return retValue;
}

void ui_paint_transition(ui_paint& target, ui_paint const& left, ui_paint const& right, f64 step)
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
            grad.Feather = length::Lerp(lc->Radius, rc->Radius, step);
            grad.Colors  = gfx::color_gradient::Lerp(lc->Colors, rc->Colors, step);
            target       = grad;
        }
        return;
    }
}

} // namespace ui
