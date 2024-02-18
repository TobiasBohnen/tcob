// Copyright (c) 2023 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/ui/widgets/Widget.hpp"

#include "tcob/gfx/ui/Form.hpp"
#include "tcob/gfx/ui/WidgetPainter.hpp"
#include "tcob/gfx/ui/widgets/Tooltip.hpp"

namespace tcob::gfx::ui {

widget::widget(init const& wi)
    : Alpha {{[&]() {
                  f32 retValue {_alpha};
                  if (auto* parent {get_parent()}) {
                      retValue *= parent->Alpha;
                  }
                  return retValue;
              },
              [&](auto const& value) { _alpha = value; }}}
    , _form {wi.Form}
    , _parent {wi.Parent}
    , _name {wi.Name}
{
    Class.Changed.connect([&](auto const&) { force_redraw(_name + ": Class changed"); });
    Bounds.Changed.connect([&](auto const&) { on_bounds_changed(); });

    Flex.Changed.connect([&](auto const&) { force_redraw(_name + ": Flex changed"); });

    static i32 tabIndex {0};
    (*TabStop).Index = tabIndex++;
}

void widget::on_bounds_changed()
{
    force_redraw(_name + ": Bounds changed");
}

void widget::show()
{
    if (!_visible) {
        _visible = true;
        force_redraw(_name + ": Visibility changed");
    }
}

void widget::hide()
{
    if (_visible) {
        _visible = false;
        force_redraw(_name + ": Visibility changed");
    }
}

auto widget::is_visible() const -> bool
{
    if (_visible && Alpha() > 0.01f) {
        if (_parent) {
            return _parent->is_visible();
        }

        return _form->is_visible();
    }
    return false;
}

void widget::focus()
{
    _form->focus_widget(this);
}

auto widget::is_focused() const -> bool
{
    return _flags.Focus;
}

void widget::enable()
{
    if (!_enabled) {
        _enabled = true;
        force_redraw(_name + ": Enable changed");
    }
}

void widget::disable()
{
    if (_enabled) {
        _enabled = false;
        force_redraw(_name + ": Enable changed");
    }
}

auto widget::is_enabled() const -> bool
{
    if (_enabled) {
        if (_parent) {
            return _parent->is_enabled();
        }

        return true;
    }
    return false;
}

void widget::paint(widget_painter& painter)
{
    if (!is_visible()) {
        return;
    }

    painter.begin(Alpha);

    on_paint(painter);

    painter.end();
}

void widget::update(milliseconds deltaTime)
{
    on_update(deltaTime);
}

auto widget::hit_test(point_f pos) const -> bool
{
    if (is_visible() && is_enabled()) {
        if (get_hit_test_bounds().contains(pos)) {
            return true;
        }
    }

    return false;
}

auto widget::get_hit_test_bounds() const -> rect_f
{
    rect_f retValue {get_global_position(), Bounds->get_size()};
    offset_content(retValue, true);

    if (_parent) {
        retValue = retValue.as_intersection(_parent->get_global_content_bounds());
    }
    if (_form) {
        retValue = retValue.as_intersection(_form->Bounds);
    }

    return retValue;
}

auto widget::get_global_position() const -> point_f
{
    auto retValue {Bounds->get_position()};
    if (_parent) {
        retValue += _parent->get_global_content_bounds().get_position() - _parent->get_scroll_offset();
    } else if (_form) {
        retValue += _form->Bounds->get_position();
    }
    return retValue;
}

auto widget::get_global_content_bounds() const -> rect_f
{
    rect_f retValue {get_global_position(), Bounds->get_size()};
    offset_content(retValue, false);
    return retValue;
}

auto widget::get_content_bounds() const -> rect_f
{
    rect_f retValue {Bounds()};
    offset_content(retValue, false);
    return retValue;
}

auto widget::global_to_local(point_i p) const -> point_f
{
    return point_f {p} - get_global_content_bounds().get_position();
}

auto widget::global_to_parent_local(point_i p) const -> point_f
{
    point_f retValue {p};
    if (_parent) {
        retValue -= (_parent->get_global_content_bounds().get_position() - _parent->get_scroll_offset());
    } else if (_form) {
        retValue -= point_f {_form->Bounds->get_position()};
    }

    return retValue;
}

void widget::offset_content(rect_f& bounds, bool isHitTest) const
{
    if (_style) {
        _style->offset_content(bounds, isHitTest);
    }
}

auto widget::get_scroll_offset() const -> point_f
{
    return point_f::Zero;
}

void widget::update_style()
{
    _style = _form->Styles->get(Class(), get_flags(), get_attributes());

    if (Tooltip) {
        Tooltip->update_style();
    }
}

auto widget::can_tab_stop() const -> bool
{
    return TabStop->Enabled && is_enabled() && is_visible();
}

auto widget::find_child_at(point_f /* pos */) -> std::shared_ptr<widget>
{
    return nullptr;
}

auto widget::find_child_by_name(string const& /* name */) -> std::shared_ptr<widget>
{
    return nullptr;
}

void widget::force_redraw(string const& reason)
{
    // TODO: limit redraw to affected widgets
    if (_parent) {
        _parent->force_redraw(reason);
    } else {
        _form->force_redraw(reason);
    }
}

auto widget::get_parent() const -> widget_container*
{
    return dynamic_cast<widget_container*>(_parent);
}

auto widget::get_form() const -> form*
{
    return _form;
}

auto widget::get_name() const -> string const&
{
    return _name;
}

void widget::do_key_down(input::keyboard::event& ev)
{
    on_key_down(ev);

    if (!ev.Handled) {
        auto const& controls {_form->Controls};
        if (!input::system::IsKeyDown(controls->ActivateKey)) {
            if (ev.KeyCode == controls->NavLeftKey) {
                _form->focus_nav_target(_name, direction::Left);
            } else if (ev.KeyCode == controls->NavRightKey) {
                _form->focus_nav_target(_name, direction::Right);
            } else if (ev.KeyCode == controls->NavDownKey) {
                _form->focus_nav_target(_name, direction::Down);
            } else if (ev.KeyCode == controls->NavUpKey) {
                _form->focus_nav_target(_name, direction::Up);
            }
        } else if (ev.KeyCode == controls->ActivateKey) {
            activate();
        }
    }

    ev.Handled = true;
    KeyDown({this, ev});
}

void widget::do_key_up(input::keyboard::event& ev)
{
    on_key_up(ev);

    if (ev.KeyCode == _form->Controls->ActivateKey) {
        deactivate();
        do_click();
    }

    ev.Handled = true;
    KeyUp({this, ev});
}

void widget::do_text_input(input::keyboard::text_input_event& ev)
{
    on_text_input(ev);
}

void widget::do_text_editing(input::keyboard::text_editing_event& ev)
{
    on_text_editing(ev);
}

void widget::do_mouse_enter()
{
    _flags.Hover = true;

    // bubble up hover state
    auto* parent {_parent};
    while (parent) {
        parent->_flags.Hover = true;
        parent               = parent->_parent;
    }

    force_redraw(_name + ": MouseEnter");

    on_mouse_enter();
    MouseEnter({this});
}

void widget::do_mouse_leave()
{
    _flags.Hover = false;

    // bubble up hover state
    auto* parent {_parent};
    while (parent) {
        parent->_flags.Hover = false;
        parent               = parent->_parent;
    }

    force_redraw(_name + ": MouseLeave");

    on_mouse_leave();
    MouseLeave({this});
}

void widget::do_mouse_hover(input::mouse::motion_event& ev)
{
    on_mouse_hover(ev);

    ev.Handled = true;
    MouseHover({.Sender           = this,
                .RelativePosition = ev.Position - point_i {get_hit_test_bounds().get_position()},
                .Event            = ev});
}

void widget::do_mouse_drag(input::mouse::motion_event& ev)
{
    on_mouse_drag(ev);

    ev.Handled = true;
    MouseDrag({.Sender           = this,
               .RelativePosition = ev.Position - point_i {get_hit_test_bounds().get_position()},
               .Event            = ev});
}

void widget::do_mouse_down(input::mouse::button_event& ev)
{
    on_mouse_down(ev);

    if (ev.Button == get_form()->Controls->PrimaryMouseButton) {
        activate();
    }

    ev.Handled = true;
    MouseDown({.Sender           = this,
               .RelativePosition = ev.Position - point_i {get_hit_test_bounds().get_position()},
               .Event            = ev});
}

void widget::do_mouse_up(input::mouse::button_event& ev)
{
    on_mouse_up(ev);

    if (ev.Button == get_form()->Controls->PrimaryMouseButton) {
        deactivate();
    }

    ev.Handled = true;
    MouseUp({.Sender           = this,
             .RelativePosition = ev.Position - point_i {get_hit_test_bounds().get_position()},
             .Event            = ev});
}

void widget::do_mouse_wheel(input::mouse::wheel_event& ev)
{
    on_mouse_wheel(ev);

    ev.Handled = true;
    MouseWheel({this, ev});
}

void widget::do_controller_button_down(input::controller::button_event& ev)
{
    on_controller_button_down(ev);

    if (!ev.Handled) {
        auto const& controls {_form->Controls};

        if (ev.Button == controls->ActivateButton) {
            activate();
        } else if (!ev.Controller->is_button_pressed(controls->ActivateButton)) {
            if (ev.Button == controls->NavLeftButton) {
                _form->focus_nav_target(_name, direction::Left);
            } else if (ev.Button == controls->NavRightButton) {
                _form->focus_nav_target(_name, direction::Right);
            } else if (ev.Button == controls->NavDownButton) {
                _form->focus_nav_target(_name, direction::Down);
            } else if (ev.Button == controls->NavUpButton) {
                _form->focus_nav_target(_name, direction::Up);
            }
        }

        ev.Handled = true;
    }

    ControllerButtonDown({this, ev});
}

void widget::do_controller_button_up(input::controller::button_event& ev)
{
    on_controller_button_up(ev);

    if (!ev.Handled) {
        if (ev.Button == _form->Controls->ActivateButton) {
            deactivate();
            do_click();
        }
    }

    ev.Handled = true;
    ControllerButtonUp({this, ev});
}

void widget::do_click()
{
    on_click();

    Click({this});
}

void widget::do_double_click()
{
    on_double_click();

    DoubleClick({this});
}

void widget::do_focus_gained()
{
    _flags.Focus = true;
    force_redraw(_name + ": FocusGained");

    on_focus_gained();

    FocusGained({this});
}

void widget::do_focus_lost()
{
    _flags.Focus = false;
    force_redraw(_name + ": FocusLost");

    on_focus_lost();

    FocusLost({this});
}

auto widget::get_attributes() const -> widget_attributes
{
    return {{"name", _name}, {"orientation", get_orientation()}};
}

auto widget::get_properties() const -> widget_attributes
{
    return {};
}

auto widget::get_flags() -> flags
{
    _flags.Disabled = !is_enabled();
    if (_flags.Disabled) {
        _flags.Active = false;
        _flags.Focus  = false;
        _flags.Hover  = false;
    }

    return _flags;
}

auto widget::get_orientation() const -> orientation
{
    return Bounds->Width >= Bounds->Height ? orientation::Horizontal : orientation::Vertical;
}

void widget::activate()
{
    if (!_flags.Active) {
        _flags.Active = true;
        force_redraw(_name + ": Active changed");
    }
}

void widget::deactivate()
{
    if (_flags.Active) {
        _flags.Active = false;
        force_redraw(_name + ": Active changed");
    }
}

void widget::collect_widgets(std::vector<widget*>& vec)
{
    vec.push_back(this);
}

void widget::on_styles_changed()
{
}

auto widget::get_styles() const -> style_collection const&
{
    return _form->Styles();
}

////////////////////////////////////////////////////////////

scissor_guard::scissor_guard(widget_painter& painter, widget* w)
    : _painter {painter}
{
    rect_f bounds {w->get_global_content_bounds()};

    if (auto const* form {w->get_form()}) {
        point_f const off {form->Bounds->get_position()};
        bounds.X -= off.X;
        bounds.Y -= off.Y;
    }
    painter.push_scissor(bounds);
}

scissor_guard::~scissor_guard()
{
    _painter.pop_scissor();
}

}
