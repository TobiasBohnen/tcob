// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/ui/UI.hpp"

#include <algorithm>
#include <cassert>

#include "tcob/core/Common.hpp"
#include "tcob/core/Point.hpp"
#include "tcob/core/input/Input.hpp"
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

    void input_injector::on_mouse_button_down(widget* widget, input::mouse::button_event const& ev) const
    {
        if (check(widget)) { widget->do_mouse_button_down(ev); }
    }

    void input_injector::on_mouse_button_up(widget* widget, input::mouse::button_event const& ev) const
    {
        if (check(widget)) { widget->do_mouse_button_up(ev); }
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

auto length::Lerp(length const& from, length const& to, f64 step) -> length
{
    assert(from.Type == to.Type);
    length retValue;
    retValue.Type  = from.Type;
    retValue.Value = helper::lerp(from.Value, to.Value, step);
    return retValue;
}

auto thickness::Lerp(thickness const& from, thickness const& to, f64 step) -> thickness
{
    thickness retValue;
    retValue.Bottom = length::Lerp(from.Bottom, to.Bottom, step);
    retValue.Top    = length::Lerp(from.Top, to.Top, step);
    retValue.Left   = length::Lerp(from.Left, to.Left, step);
    retValue.Right  = length::Lerp(from.Right, to.Right, step);
    return retValue;
}

auto dimensions::Lerp(dimensions const& from, dimensions const& to, f64 step) -> dimensions
{
    dimensions retValue;
    retValue.Width  = length::Lerp(from.Width, to.Width, step);
    retValue.Height = length::Lerp(from.Height, to.Height, step);
    return retValue;
}

auto screen_to_content(widget const& widget, point_i p) -> point_f
{
    return point_f {p} - (widget.content_bounds().Position + widget.form_offset() + widget.form().Bounds->Position);
}

auto screen_to_local(widget const& widget, point_i p) -> point_f
{
    point_f delta {};
    if (auto* parent {widget.parent()}) {
        delta = parent->content_bounds().Position + parent->form_offset() + parent->form().Bounds->Position - parent->scroll_offset();
    } else {
        delta = widget.form().Bounds->Position;
    }

    return point_f {p} - delta;
}

auto local_to_screen(widget const& widget, point_f p) -> point_f
{
    point_f delta {};
    if (auto* parent {widget.parent()}) {
        delta = parent->content_bounds().Position + parent->form_offset() + parent->form().Bounds->Position - parent->scroll_offset();
    } else {
        delta = widget.form().Bounds->Position;
    }

    return p + delta;
}

}
