// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/ui/widgets/Widget.hpp"

#include <cassert>

#include "tcob/core/Common.hpp"
#include "tcob/core/Point.hpp"
#include "tcob/core/Rect.hpp"
#include "tcob/core/input/Input.hpp"
#include "tcob/gfx/animation/Animation.hpp"
#include "tcob/gfx/ui/Form.hpp"
#include "tcob/gfx/ui/Style.hpp"
#include "tcob/gfx/ui/StyleCollection.hpp"
#include "tcob/gfx/ui/UI.hpp"
#include "tcob/gfx/ui/WidgetPainter.hpp"
#include "tcob/gfx/ui/widgets/WidgetContainer.hpp"

namespace tcob::ui {

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
    assert(_form);

    Class.Changed.connect([this](auto const&) { request_redraw(_name + ": Class changed"); });
    Bounds.Changed.connect([this](auto const&) { on_bounds_changed(); });

    Flex.Changed.connect([this](auto const&) { request_redraw(_name + ": Flex changed"); });

    ZOrder.Changed.connect([this](auto const&) { request_redraw(_name + ": ZOrder changed"); });

    static i32 tabIndex {0};
    (*TabStop).Index = tabIndex++;

    _animationTween.Changed.connect([this](auto const& val) { on_animation_step(val); });
}

void widget::on_bounds_changed()
{
    request_redraw(_name + ": Bounds changed");
}

void widget::show()
{
    if (!_visible) {
        _visible = true;
        request_redraw(_name + ": Visibility changed");
    }
}

void widget::hide()
{
    if (_visible) {
        _visible = false;
        request_redraw(_name + ": Visibility changed");
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
        request_redraw(_name + ": Enable changed");
    }
}

void widget::disable()
{
    if (_enabled) {
        _enabled = false;
        request_redraw(_name + ": Enable changed");
    }
}

auto widget::is_enabled() const -> bool
{
    return _enabled && (_parent ? _parent->is_enabled() : true);
}

void widget::draw(widget_painter& painter)
{
    if (!_redraw) { return; }
    _redraw = false;

    if (!is_visible() || Bounds->width() <= 0 || Bounds->height() <= 0) { return; }

    painter.begin(Alpha());

    on_draw(painter);

    painter.end();
}

void widget::update(milliseconds deltaTime)
{
    if (_transition.is_active()) { request_redraw(this->name() + ": Transition"); }
    _transition.update(deltaTime);

    // item transitions
    for (auto& [_, v] : _subStyleTransitions) {
        if (v.is_active()) { request_redraw(this->name() + ": Item transition"); }
        v.update(deltaTime);
    }

    _animationTween.update(deltaTime);

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
    retValue = retValue.as_intersection_with(_form->Bounds());

    return retValue;
}

auto widget::global_position() const -> point_f
{
    auto retValue {Bounds->Position};
    if (_parent) {
        retValue += _parent->global_content_bounds().Position - _parent->scroll_offset();
    } else {
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

auto widget::can_tab_stop(i32 high, i32 low) const -> bool
{
    return TabStop->Enabled && is_enabled() && is_visible() && !is_inert() && TabStop->Index < high && TabStop->Index > low;
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

void widget::request_redraw(string const& reason)
{
    if (_redraw) { return; }

    if (is_top_level()) { // is top level widget -> redraw everything
        _form->request_redraw(reason);
    } else {
        auto* tlw {top_level_widget()};
        assert(tlw);
        for (auto const& w : _form->containers()) {
            if (w.get() == tlw) { continue; }
            if (w->ZOrder < tlw->ZOrder) { continue; }

            if (w->Bounds->intersects(tlw->Bounds())) { // top level widget overlaps with other widget -> redraw everything
                _form->request_redraw(reason);
                return;
            }
        }

        // redraw top level widget
        tlw->set_redraw(true);
        _form->notify_redraw(reason);
    }
}

void widget::set_redraw(bool val)
{
    _redraw = val;
}

auto widget::get_redraw() const -> bool
{
    return _redraw;
}

auto widget::parent() const -> widget_container*
{
    return dynamic_cast<widget_container*>(_parent);
}

auto widget::parent_form() const -> form_base*
{
    return _form;
}

auto widget::is_top_level() const -> bool
{
    return _parent == nullptr;
}

auto widget::top_level_widget() -> widget*
{
    if (_parent == nullptr) { return this; }
    return _parent->top_level_widget();
}

auto widget::name() const -> string const&
{
    return _name;
}

void widget::do_key_down(input::keyboard::event const& ev)
{
    on_key_down(ev);

    if (!ev.Handled) {
        if (ev.KeyCode == controls().ActivateKey) {
            activate();
            ev.Handled = true;
        }
    }

    if (ev.Handled) {
        request_redraw(_name + ": KeyDown");
    }

    KeyDown({this, ev});
}

void widget::do_key_up(input::keyboard::event const& ev)
{
    on_key_up(ev);

    if (ev.KeyCode == controls().ActivateKey) {
        deactivate();
        do_click();
        ev.Handled = true;
    }

    if (ev.Handled) {
        request_redraw(_name + ": KeyUp");
    }

    KeyUp({this, ev});
}

void widget::do_text_input(input::keyboard::text_input_event const& ev)
{
    on_text_input(ev);
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

    request_redraw(_name + ": MouseEnter");

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

    request_redraw(_name + ": MouseLeave");

    on_mouse_leave();
    MouseLeave({this});
}

void widget::do_mouse_hover(input::mouse::motion_event const& ev)
{
    on_mouse_hover(ev);

    if (ev.Handled) {
        request_redraw(_name + ": MouseHover");
    }

    MouseHover({.Sender           = this,
                .RelativePosition = ev.Position - point_i {hit_test_bounds().Position},
                .Event            = ev});
}

void widget::do_mouse_drag(input::mouse::motion_event const& ev)
{
    on_mouse_drag(ev);

    if (ev.Handled) {
        request_redraw(_name + ": MouseDrag");
    }

    MouseDrag({.Sender           = this,
               .RelativePosition = ev.Position - point_i {hit_test_bounds().Position},
               .Event            = ev});
}

void widget::do_mouse_down(input::mouse::button_event const& ev)
{
    on_mouse_button_down(ev);

    if (ev.Button == controls().PrimaryMouseButton) {
        activate();
        ev.Handled = true;
    }

    if (ev.Handled) {
        request_redraw(_name + ": MouseDown");
    }

    MouseDown({.Sender           = this,
               .RelativePosition = ev.Position - point_i {hit_test_bounds().Position},
               .Event            = ev});
}

void widget::do_mouse_up(input::mouse::button_event const& ev)
{
    on_mouse_button_up(ev);

    if (ev.Button == controls().PrimaryMouseButton) {
        deactivate();
        ev.Handled = true;
    }

    if (ev.Handled) {
        request_redraw(_name + ": MouseUp");
    }

    MouseUp({.Sender           = this,
             .RelativePosition = ev.Position - point_i {hit_test_bounds().Position},
             .Event            = ev});
}

void widget::do_mouse_wheel(input::mouse::wheel_event const& ev)
{
    on_mouse_wheel(ev);

    if (ev.Handled) {
        request_redraw(_name + ": MouseWheel");
    }

    MouseWheel({this, ev});
}

void widget::do_controller_button_down(input::controller::button_event const& ev)
{
    on_controller_button_down(ev);

    if (!ev.Handled) {
        if (ev.Button == controls().ActivateButton) {
            activate();
            ev.Handled = true;
        }
    }

    if (ev.Handled) {
        request_redraw(_name + ": ControllerButtonDown");
    }

    ControllerButtonDown({this, ev});
}

void widget::do_controller_button_up(input::controller::button_event const& ev)
{
    on_controller_button_up(ev);

    if (!ev.Handled) {
        if (ev.Button == controls().ActivateButton) {
            deactivate();
            do_click();
            ev.Handled = true;
        }
    }

    if (ev.Handled) {
        request_redraw(_name + ": ControllerButtonUp");
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
    request_redraw(_name + ": FocusGained");

    on_focus_gained();

    FocusGained({this});
}

void widget::do_focus_lost()
{
    _flags.Focus = false;
    request_redraw(_name + ": FocusLost");

    on_focus_lost();

    FocusLost({this});
}

auto widget::attributes() const -> widget_attributes
{
    return {{"name", _name}, {"orientation", get_orientation()}};
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

auto widget::get_orientation() const -> orientation
{
    return Bounds->width() >= Bounds->height() ? orientation::Horizontal : orientation::Vertical;
}

auto widget::is_inert() const -> bool
{
    return false;
}

void widget::start_animation(gfx::frame_animation const& ani, playback_mode mode)
{
    _animationTween.start(ani, mode);
}

void widget::stop_animation()
{
    _animationTween.stop();
}

void widget::on_animation_step(string const& /* val */)
{
}

void widget::activate()
{
    if (!_flags.Active) {
        _flags.Active = true;
        request_redraw(_name + ": Active changed");
    }
}

void widget::deactivate()
{
    if (_flags.Active) {
        _flags.Active = false;
        request_redraw(_name + ": Active changed");
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

auto widget::controls() const -> control_map const&
{
    return _form->Controls();
}
}
