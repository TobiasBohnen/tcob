// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/ui/widgets/Widget.hpp"

#include "tcob/core/ServiceLocator.hpp"
#include "tcob/gfx/ui/Form.hpp"
#include "tcob/gfx/ui/WidgetPainter.hpp"
#include "tcob/gfx/ui/widgets/WidgetContainer.hpp"

namespace tcob::gfx::ui {

widget::widget(init const& wi)
    : Alpha {{[this]() {
                  f32 retValue {_alpha};
                  if (auto* wparent {parent()}) { retValue *= wparent->Alpha(); }
                  return retValue;
              },
              [this](auto const& value) { _alpha = value; }}}
    , _form {wi.Form}
    , _parent {wi.Parent}
    , _name {wi.Name}
{
    Class.Changed.connect([this](auto const&) { force_redraw(_name + ": Class changed"); });
    Bounds.Changed.connect([this](auto const&) { on_bounds_changed(); });

    Flex.Changed.connect([this](auto const&) { force_redraw(_name + ": Flex changed"); });

    ZOrder.Changed.connect([this](auto const&) { force_redraw(_name + ": ZOrder changed"); });

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
    return _visible && Alpha() > 0.01f
        && (_parent ? _parent->is_visible()
                    : _form->is_visible());
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
    return _enabled && (_parent ? _parent->is_enabled() : true);
}

void widget::paint(widget_painter& painter)
{
    if (!is_visible() || Bounds->width() <= 0 || Bounds->height() <= 0) { return; }

    painter.begin(Alpha());

    on_paint(painter);

    painter.end();
}

void widget::update(milliseconds deltaTime)
{
    if (_transition.is_active()) { force_redraw(this->name() + ": Transition"); }
    _transition.update(deltaTime);

    // item transitions
    for (auto& [_, v] : _subStyleTransitions) {
        if (v.is_active()) { force_redraw(this->name() + ": Item transition"); }
        v.update(deltaTime);
    }

    on_update(deltaTime);
}

auto widget::hit_test(point_f pos) const -> bool
{
    return is_visible()
        && is_enabled()
        && hit_test_bounds().contains(pos);
}

auto widget::current_style() const -> widget_style const*
{
    return _currentStyle;
}

auto widget::hit_test_bounds() const -> rect_f
{
    rect_f retValue {global_position(), Bounds->Size};
    offset_content(retValue, true);

    if (_parent) { retValue = retValue.as_intersection_with(_parent->global_content_bounds()); }
    if (_form) { retValue = retValue.as_intersection_with(_form->Bounds()); }

    return retValue;
}

auto widget::global_position() const -> point_f
{
    auto retValue {Bounds->Position};
    if (_parent) {
        retValue += _parent->global_content_bounds().Position - _parent->scroll_offset();
    } else if (_form) {
        retValue += _form->Bounds->Position;
    }
    return retValue;
}

auto widget::global_content_bounds() const -> rect_f
{
    rect_f retValue {global_position(), Bounds->Size};
    offset_content(retValue, false);
    return retValue;
}

auto widget::content_bounds() const -> rect_f
{
    rect_f retValue {Bounds()};
    offset_content(retValue, false);
    return retValue;
}

auto widget::global_to_content(point_i p) const -> point_f
{
    return point_f {p} - global_content_bounds().Position;
}

auto widget::global_to_local(point_i p) const -> point_f
{
    point_f retValue {p};
    if (_parent) {
        retValue -= (_parent->global_content_bounds().Position - _parent->scroll_offset());
    } else if (_form) {
        retValue -= point_f {_form->Bounds->Position};
    }

    return retValue;
}

void widget::offset_content(rect_f& bounds, bool isHitTest) const
{
    if (!_currentStyle) { return; }

    bounds -= _currentStyle->Margin;
    if (!isHitTest) {
        bounds -= _currentStyle->Padding;
        bounds -= _currentStyle->Border.thickness();
    }
}

auto widget::scroll_offset() const -> point_f
{
    return point_f::Zero;
}

auto widget::can_tab_stop() const -> bool
{
    return TabStop->Enabled && is_enabled() && is_visible() && !is_inert();
}

void widget::on_styles_changed()
{
    widget_style_selectors const newSelectors {
        .Class      = Class(),
        .Flags      = flags(),
        .Attributes = attributes(),
    };

    auto* style {dynamic_cast<widget_style*>(_form->Styles->get(newSelectors))};
    _transition.reset(style);
    _currentStyle = style;

    _lastSelectors = newSelectors;

    _subStyleTransitions.clear();
}

void widget::prepare_redraw()
{
    widget_style_selectors const newSelectors {
        .Class      = Class(),
        .Flags      = flags(),
        .Attributes = attributes(),
    };

    if (_lastSelectors != newSelectors) {
        auto* style {dynamic_cast<widget_style*>(_form->Styles->get(newSelectors))};
        _transition.try_start(style, TransitionDuration);
        _currentStyle = style;

        _lastSelectors = newSelectors;
    }
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

auto widget::parent() const -> widget_container*
{
    return dynamic_cast<widget_container*>(_parent);
}

auto widget::parent_form() const -> form*
{
    return _form;
}

auto widget::name() const -> string const&
{
    return _name;
}

void widget::do_key_down(input::keyboard::event const& ev)
{
    on_key_down(ev);

    if (!ev.Handled) {
        auto const& controls {_form->Controls};
        if (!locate_service<input::system>().keyboard().is_key_down(controls->ActivateKey)) {
            if (ev.KeyCode == controls->NavLeftKey) {
                ev.Handled = _form->focus_nav_target(_name, direction::Left);
            } else if (ev.KeyCode == controls->NavRightKey) {
                ev.Handled = _form->focus_nav_target(_name, direction::Right);
            } else if (ev.KeyCode == controls->NavDownKey) {
                ev.Handled = _form->focus_nav_target(_name, direction::Down);
            } else if (ev.KeyCode == controls->NavUpKey) {
                ev.Handled = _form->focus_nav_target(_name, direction::Up);
            }
        } else if (ev.KeyCode == controls->ActivateKey) {
            activate();
            ev.Handled = true;
        }
    }

    KeyDown({this, ev});
}

void widget::do_key_up(input::keyboard::event const& ev)
{
    on_key_up(ev);

    if (ev.KeyCode == _form->Controls->ActivateKey) {
        deactivate();
        do_click();
        ev.Handled = true;
    }

    KeyUp({this, ev});
}

void widget::do_text_input(input::keyboard::text_input_event const& ev)
{
    on_text_input(ev);
}

void widget::do_text_editing(input::keyboard::text_editing_event const& ev)
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

void widget::do_mouse_hover(input::mouse::motion_event const& ev)
{
    on_mouse_hover(ev);

    MouseHover({.Sender           = this,
                .RelativePosition = ev.Position - point_i {hit_test_bounds().Position},
                .Event            = ev});
}

void widget::do_mouse_drag(input::mouse::motion_event const& ev)
{
    on_mouse_drag(ev);

    MouseDrag({.Sender           = this,
               .RelativePosition = ev.Position - point_i {hit_test_bounds().Position},
               .Event            = ev});
}

void widget::do_mouse_down(input::mouse::button_event const& ev)
{
    on_mouse_down(ev);

    if (ev.Button == parent_form()->Controls->PrimaryMouseButton) {
        activate();
        ev.Handled = true;
    }

    MouseDown({.Sender           = this,
               .RelativePosition = ev.Position - point_i {hit_test_bounds().Position},
               .Event            = ev});
}

void widget::do_mouse_up(input::mouse::button_event const& ev)
{
    on_mouse_up(ev);

    if (ev.Button == parent_form()->Controls->PrimaryMouseButton) {
        deactivate();
        ev.Handled = true;
    }

    MouseUp({.Sender           = this,
             .RelativePosition = ev.Position - point_i {hit_test_bounds().Position},
             .Event            = ev});
}

void widget::do_mouse_wheel(input::mouse::wheel_event const& ev)
{
    on_mouse_wheel(ev);

    MouseWheel({this, ev});
}

void widget::do_controller_button_down(input::controller::button_event const& ev)
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

void widget::do_controller_button_up(input::controller::button_event const& ev)
{
    on_controller_button_up(ev);

    if (!ev.Handled) {
        if (ev.Button == _form->Controls->ActivateButton) {
            deactivate();
            do_click();
            ev.Handled = true;
        }
    }

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

auto widget::attributes() const -> widget_attributes
{
    return {{"name", _name}, {"orientation", current_orientation()}};
}

auto widget::flags() -> widget_flags
{
    _flags.Disabled = !is_enabled();
    if (_flags.Disabled) {
        _flags.Active = false;
        _flags.Focus  = false;
        _flags.Hover  = false;
    }

    return _flags;
}

auto widget::current_orientation() const -> orientation
{
    return Bounds->width() >= Bounds->height() ? orientation::Horizontal : orientation::Vertical;
}

auto widget::is_inert() const -> bool
{
    return false;
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

auto widget::styles() const -> style_collection const&
{
    return _form->Styles();
}

void widget::reset_sub_style(isize idx, string const& styleClass, widget_flags flags)
{
    widget_style_selectors const selectors {
        .Class      = styleClass,
        .Flags      = flags,
        .Attributes = attributes(),
    };
    auto* subStyle {styles().get(selectors)};
    _subStyleTransitions[idx].reset(subStyle);
}

void widget::clear_sub_styles()
{
    _subStyleTransitions.clear();
}

}
