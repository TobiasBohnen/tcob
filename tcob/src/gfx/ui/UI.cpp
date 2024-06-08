// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/ui/UI.hpp"

#include "tcob/gfx/ui/widgets/Widget.hpp"

namespace tcob::gfx::ui {

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
    case type::Absolute:
        return Value;
    case type::Relative:
        return Value * refSize;
    }

    return Value;
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

auto flags::check(flags other) const -> i32
{
    i32 retValue {0};

    auto const myBits {as_array()};
    auto const otherBits {other.as_array()};

    for (u32 i {0}; i < Size; ++i) {
        if (myBits[i] && !otherBits[i]) {
            return std::numeric_limits<i32>::min();
        }
        if (myBits[i] && otherBits[i]) {
            ++retValue;
        }
    }

    return retValue;
}

auto flags::as_array() const -> std::array<bool, Size>
{
    return {Focus, Hover, Active, Checked, Disabled};
}

////////////////////////////////////////////////////////////

namespace detail {
    void input_injector::on_key_down(widget* widget, input::keyboard::event& ev)
    {
        if (widget && widget->is_enabled() && !widget->is_inert()) {
            widget->do_key_down(ev);
        }
    }

    void input_injector::on_key_up(widget* widget, input::keyboard::event& ev)
    {
        if (widget && widget->is_enabled() && !widget->is_inert()) {
            widget->do_key_up(ev);
        }
    }

    void input_injector::on_text_input(widget* widget, input::keyboard::text_input_event& ev)
    {
        if (widget && widget->is_enabled() && !widget->is_inert()) {
            widget->do_text_input(ev);
        }
    }

    void input_injector::on_text_editing(widget* widget, input::keyboard::text_editing_event& ev)
    {
        if (widget && widget->is_enabled() && !widget->is_inert()) {
            widget->do_text_editing(ev);
        }
    }

    void input_injector::on_mouse_enter(widget* widget)
    {
        if (widget && widget->is_enabled() && !widget->is_inert()) {
            widget->do_mouse_enter();
        }
    }

    void input_injector::on_mouse_leave(widget* widget)
    {
        if (widget && widget->is_enabled() && !widget->is_inert()) {
            widget->do_mouse_leave();
        }
    }

    void input_injector::on_mouse_down(widget* widget, input::mouse::button_event& ev)
    {
        if (widget && widget->is_enabled() && !widget->is_inert()) {
            widget->do_mouse_down(ev);
        }
    }

    void input_injector::on_mouse_up(widget* widget, input::mouse::button_event& ev)
    {
        if (widget && widget->is_enabled() && !widget->is_inert()) {
            widget->do_mouse_up(ev);
        }
    }

    void input_injector::on_mouse_hover(widget* widget, input::mouse::motion_event& ev)
    {
        if (widget && widget->is_enabled() && !widget->is_inert()) {
            widget->do_mouse_hover(ev);
        }
    }

    void input_injector::on_mouse_drag(widget* widget, input::mouse::motion_event& ev)
    {
        if (widget && widget->is_enabled() && !widget->is_inert()) {
            widget->do_mouse_drag(ev);
        }
    }

    void input_injector::on_mouse_wheel(widget* widget, input::mouse::wheel_event& ev)
    {
        if (widget && widget->is_enabled() && !widget->is_inert()) {
            widget->do_mouse_wheel(ev);
        }
    }

    void input_injector::on_controller_button_down(widget* widget, input::controller::button_event& ev)
    {
        if (widget && widget->is_enabled() && !widget->is_inert()) {
            widget->do_controller_button_down(ev);
        }
    }

    void input_injector::on_controller_button_up(widget* widget, input::controller::button_event& ev)
    {
        if (widget && widget->is_enabled() && !widget->is_inert()) {
            widget->do_controller_button_up(ev);
        }
    }

    void input_injector::on_click(widget* widget)
    {
        if (widget && widget->is_enabled() && !widget->is_inert()) {
            widget->do_click();
        }
    }

    void input_injector::on_double_click(widget* widget)
    {
        if (widget && widget->is_enabled() && !widget->is_inert()) {
            widget->do_double_click();
        }
    }

    void input_injector::on_focus_gained(widget* widget)
    {
        if (widget && widget->is_enabled() && !widget->is_inert()) {
            widget->do_focus_gained();
        }
    }

    void input_injector::on_focus_lost(widget* widget)
    {
        if (widget && widget->is_enabled() && !widget->is_inert()) {
            widget->do_focus_lost();
        }
    }

}

} // namespace ui
