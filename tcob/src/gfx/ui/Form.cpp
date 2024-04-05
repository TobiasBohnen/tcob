// Copyright (c) 2023 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/ui/Form.hpp"

#include <utility>

#include "tcob/core/Logger.hpp"
#include "tcob/core/ServiceLocator.hpp"
#include "tcob/gfx/Geometry.hpp"
#include "tcob/gfx/ui/WidgetPainter.hpp"
#include "tcob/gfx/ui/widgets/Tooltip.hpp"

namespace tcob::gfx::ui {

using namespace std::chrono_literals;

////////////////////////////////////////////////////////////

form::form(string name, window* window)
    : form {std::move(name), window, rect_f {point_f::Zero, window ? size_f {window->Size()} : size_f::Zero}}
{
}

form::form(string name, window* window, rect_f const& bounds)
    : Bounds {bounds}
    , _window {window}
    , _layout {this}
    , _painter {std::make_unique<widget_painter>(_canvas)}
    , _name {std::move(name)}
{
    Scale(size_f::One);
    Scale.Changed.connect([&](auto const&) { on_bounds_changed(); });
    Bounds.Changed.connect([&](auto const&) { on_bounds_changed(); });
    on_bounds_changed();

    Styles.Changed.connect([&](auto const&) { on_styles_changed(); });
}

form::~form()
{
    // TODO: disconnect ALL events
}

auto form::get_name() const -> string const&
{
    return _name;
}

auto form::get_top_widget() const -> widget*
{
    return _topWidget;
}

auto form::find_widget_at(point_f pos) -> std::shared_ptr<widget>
{
    for (auto& container : _layout.get_widgets()) {
        if (container->hit_test(pos)) {
            if (auto retValue {container->find_child_at(pos)}) {
                return retValue;
            }
            return container;
        }
    }
    return nullptr;
}

auto form::find_widget_by_name(string const& name) -> std::shared_ptr<widget>
{
    for (auto& container : _layout.get_widgets()) {
        if (container->get_name() == name) {
            return container;
        }
        if (auto retValue {container->find_child_by_name(name)}) {
            return retValue;
        }
    }
    return nullptr;
}

auto form::get_widgets() const -> std::vector<std::shared_ptr<widget>> const&
{
    return _layout.get_widgets();
}

auto form::get_all_widgets() -> std::vector<widget*>
{
    std::vector<widget*> retValue;
    for (auto& container : _layout.get_widgets()) {
        container->collect_widgets(retValue);
    }
    return retValue;
}

void form::clear()
{
    _layout.clear();
    force_redraw("clearing");
}

void form::force_redraw(string const& reason)
{
    if (!_redrawWidgets) {
        logger::Debug("Form: {} redraw; reason: {}", _name, reason);
    }
    _updateWidgets = true;
    _layout.mark_dirty(); // FIXME: only if top-level controls change
}

auto form::focus_nav_target(string const& widget, direction dir) -> bool
{
    if (!NavMap->contains(widget)) {
        return false;
    }

    string navTarget;
    switch (dir) {
    case direction::Left:
        navTarget = (*NavMap)[widget].Left;
        break;
    case direction::Right:
        navTarget = (*NavMap)[widget].Right;
        break;
    case direction::Up:
        navTarget = (*NavMap)[widget].Up;
        break;
    case direction::Down:
        navTarget = (*NavMap)[widget].Down;
        break;
    case direction::None: break;
    }

    if (!navTarget.empty()) {
        if (auto target {find_widget_by_name(navTarget)}) {
            focus_widget(target.get());
            return true;
        }
    }

    return false;
}

auto form::get_update_mode() const -> update_mode
{
    return update_mode::Fixed;
}

void form::on_update(milliseconds /* deltaTime */)
{
}

void form::on_fixed_update(milliseconds deltaTime)
{
    auto const& widgets {_layout.get_widgets()};

    // tooltip
    if (_topWidget) {
        _mouseOverTime += deltaTime;
        if (can_popup_tooltip()) {
            _topWidget->Tooltip->on_popup(_topWidget);
            _isTooltipVisible = true;
        }

        if (_isTooltipVisible) {
            _topWidget->Tooltip->update(deltaTime);
        }
    }

    // update styles
    if (_updateWidgets) {
        for (auto const& container : widgets) {
            container->update_style();
        }

        // layout
        _layout.update();

        _updateWidgets = false;
        _redrawWidgets = true;
    }

    // update widgets
    for (auto const& container : widgets) {
        container->update(deltaTime);
    }
}

void form::on_styles_changed()
{
    _updateWidgets = true;
    for (auto& container : _layout.get_widgets()) {
        container->on_styles_changed();
    }
}

auto form::can_draw() const -> bool
{
    return true;
}

void form::on_draw_to(render_target& target)
{
    // set cursor
    if (_window && _window->Cursor() && _topWidget && _topWidget->_style) {
        _window->Cursor->ActiveMode = _topWidget->get_style<style>()->Cursor;
    }

    size_i const bounds {size_i {Bounds->get_size()}};

    // redraw
    if (_redrawWidgets) {
        _canvas.begin_frame(bounds, 1.0f, 0);

        for (auto& container : _layout.get_widgets()) {
            _canvas.reset();
            container->paint(*_painter);
        }

        _canvas.end_frame();
        _redrawWidgets = false;
    }

    // tooltip
    if (_isTooltipVisible) {
        point_i pos;
        if (_window && _window->Cursor()) {
            pos = static_cast<point_i>(_window->Cursor->get_bounds().get_center());
        } else {
            pos = input::system::GetMousePosition();
        }
        pos.X -= static_cast<i32>(Bounds->X);
        pos.Y -= static_cast<i32>(Bounds->Y);
        pos = scale_mouse(pos);

        (*_topWidget->Tooltip->Bounds).X = static_cast<f32>(pos.X);
        (*_topWidget->Tooltip->Bounds).Y = static_cast<f32>(pos.Y);

        _canvas.begin_frame(bounds, 1.0f, 1);
        _topWidget->Tooltip->paint(*_painter);
        _canvas.end_frame();
    }

    // render
    // store old camera and set new identity camera
    auto const oldCam {*target.Camera};
    _camera.set_size(oldCam.get_size());
    target.Camera = _camera;

    _material->Texture = _canvas.get_texture(0);
    _renderer.render_to_target(target);

    if (_isTooltipVisible) {
        _material->Texture = _canvas.get_texture(1);
        _renderer.render_to_target(target);
    }

    // restore old camera
    target.Camera = oldCam;
}

auto form::get_focus_widget() const -> widget*
{
    return _focusWidget;
}

void form::focus_widget(widget* newFocus)
{
    if (newFocus != _focusWidget) {
        _currentTabIndex = -1;
        _injector.on_focus_lost(_focusWidget);

        _focusWidget = newFocus;

        if (_focusWidget) {
            if (_focusWidget->is_inert()) {
                _focusWidget = nullptr;
                return;
            }

            _currentTabIndex = _focusWidget->TabStop->Index;
            _injector.on_focus_gained(_focusWidget);
        }
    }
}

void form::find_top_widget(input::mouse::motion_event& ev)
{
    auto* newTop {find_widget_at(point_f {ev.Position}).get()};
    if (newTop && newTop->is_inert()) {
        _injector.on_mouse_leave(_topWidget);
        _topWidget = nullptr;
        return;
    }

    if (newTop != _topWidget) {
        hide_tooltip();
        _injector.on_mouse_leave(_topWidget);
        _topWidget = newTop;
        _injector.on_mouse_enter(_topWidget);
    } else if (_topWidget) {
        _injector.on_mouse_hover(_topWidget, ev);
    }
}

void form::on_key_down(input::keyboard::event& ev)
{
    hide_tooltip();

    using namespace tcob::enum_ops;

    if (ev.KeyCode == Controls->TabKey) {
        auto const vec {get_all_widgets()};

        if ((ev.KeyMods & Controls->TabMod) == Controls->TabMod) {
            widget* nextWidget {find_prev_tab_widget(vec)};
            if (!nextWidget) {
                _currentTabIndex = std::numeric_limits<i32>::max();
                nextWidget       = find_prev_tab_widget(vec);
            }
            focus_widget(nextWidget);
        } else {
            widget* nextWidget {find_next_tab_widget(vec)};
            if (!nextWidget) {
                _currentTabIndex = -1;
                nextWidget       = find_next_tab_widget(vec);
            }
            focus_widget(nextWidget);
        }
    } else if ((ev.KeyMods & Controls->CutCopyPasteMod) == Controls->CutCopyPasteMod) {
        if (ev.KeyCode == Controls->PasteKey) {
            input::keyboard::text_input_event tev {.Text = input::system::GetClipboardText()};
            _injector.on_text_input(_focusWidget, tev);
        }
    } else {
        _injector.on_key_down(_focusWidget, ev);
    }
}

void form::on_key_up(input::keyboard::event& ev)
{
    hide_tooltip();
    _injector.on_key_up(_focusWidget, ev);
}

void form::on_mouse_motion(input::mouse::motion_event& mev)
{
    auto ev {mev};
    ev.Position = scale_mouse(ev.Position);

    if (_isLButtonDown) {
        _injector.on_mouse_drag(_focusWidget, ev);
    } else {
        find_top_widget(ev);
    }

    mev.Handled = ev.Handled;
}

void form::on_mouse_button_down(input::mouse::button_event& mev)
{
    auto ev {mev};
    ev.Position = scale_mouse(ev.Position);

    hide_tooltip();

    focus_widget(_topWidget);
    if (_topWidget) {
        _injector.on_mouse_down(_topWidget, ev);
        if (ev.Button == Controls->PrimaryMouseButton) {
            _isLButtonDown = true;
        } else if (ev.Button == Controls->SecondaryMouseButton) {
            _isRButtonDown = true;
        }
    }

    mev.Handled = ev.Handled;
}

void form::on_mouse_button_up(input::mouse::button_event& mev)
{
    auto ev {mev};
    ev.Position = scale_mouse(ev.Position);

    hide_tooltip();

    if (_focusWidget) {
        _injector.on_mouse_up(_focusWidget, ev);

        if (_topWidget == _focusWidget) {
            if (ev.Button == Controls->PrimaryMouseButton) {
                _injector.on_click(_focusWidget);

                if (ev.Clicks == 1) {
                    _clickPos = ev.Position;
                }
                if (ev.Clicks == 2 && _clickPos.distance_to(mev.Position) <= 5) {
                    _injector.on_double_click(_focusWidget);
                }
            }
        }

        if (ev.Button == Controls->PrimaryMouseButton) {
            _isLButtonDown = false;
        } else if (ev.Button == Controls->SecondaryMouseButton) {
            _isRButtonDown = false;
        }
    }

    mev.Handled = ev.Handled;
}

void form::on_mouse_wheel(input::mouse::wheel_event& mev)
{
    auto ev {mev};
    ev.Position = scale_mouse(ev.Position);

    hide_tooltip();

    if (_topWidget) {
        _injector.on_mouse_wheel(_topWidget, ev);
    } else if (_focusWidget) {
        _injector.on_mouse_wheel(_focusWidget, ev);
    }

    mev.Handled = ev.Handled;
}

void form::on_controller_axis_motion(input::controller::axis_event& /*ev*/)
{
}

void form::on_controller_button_down(input::controller::button_event& ev)
{
    hide_tooltip();
    _injector.on_controller_button_down(_focusWidget, ev);
}

void form::on_controller_button_up(input::controller::button_event& ev)
{
    hide_tooltip();
    _injector.on_controller_button_up(_focusWidget, ev);
}

void form::on_bounds_changed()
{
    quad   q {};
    rect_f bounds {Bounds};
    if (Scale != size_f::One) {
        bounds.X *= Scale->Width;
        bounds.Width *= Scale->Width;
        bounds.Y *= Scale->Height;
        bounds.Height *= Scale->Height;
    }
    geometry::set_position(q, bounds);
    geometry::set_color(q, colors::White);
    geometry::set_texcoords(q, {render_texture::GetTexcoords(), 0});
    _renderer.set_geometry(q);
    _renderer.set_material(_material);

    force_redraw("bounds changed");
    on_styles_changed();
}

void form::on_visiblity_changed()
{
    _isLButtonDown = false;
    _isRButtonDown = false;

    if (!is_visible()) {
        focus_widget(nullptr);
        if (_topWidget) {
            _injector.on_mouse_leave(_topWidget);
            _topWidget = nullptr;
        }
    }
    // TODO: else inject mouse_motion
}

void form::on_text_input(input::keyboard::text_input_event& ev)
{
    _injector.on_text_input(_focusWidget, ev);
}

void form::on_text_editing(input::keyboard::text_editing_event& ev)
{
    _injector.on_text_editing(_focusWidget, ev);
}

auto form::find_next_tab_widget(std::vector<widget*> const& vec) const -> widget*
{
    widget* retValue {nullptr};
    i32     lowestHigherValue {std::numeric_limits<i32>::max()};
    for (auto* widget : vec) {
        if (widget->can_tab_stop()
            && widget->TabStop->Index > _currentTabIndex
            && widget->TabStop->Index < lowestHigherValue) {
            lowestHigherValue = widget->TabStop->Index;
            retValue          = widget;
        }
    }
    return retValue;
}

auto form::find_prev_tab_widget(std::vector<widget*> const& vec) const -> widget*
{
    widget* retValue {nullptr};
    i32     highestLowerValue {std::numeric_limits<i32>::min()};
    for (auto* widget : vec) {
        if (widget->can_tab_stop()
            && widget->TabStop->Index < _currentTabIndex
            && widget->TabStop->Index > highestLowerValue) {
            highestLowerValue = widget->TabStop->Index;
            retValue          = widget;
        }
    }
    return retValue;
}

auto form::can_popup_tooltip() const -> bool
{
    if (!_isTooltipVisible
        && _topWidget && _topWidget->Tooltip && _focusWidget != _topWidget
        && !_isLButtonDown && !_isRButtonDown
        && locate_service<input::system>().CurrentInputMode == input::mode::KeyboardMouse) {
        if (auto style {_topWidget->Tooltip->get_style<tooltip::style>()}) {
            return _mouseOverTime > style->Delay;
        }
    }

    return false;
}

void form::hide_tooltip()
{
    _mouseOverTime    = 0ms;
    _isTooltipVisible = false;
}

auto form::scale_mouse(point_i mp) const -> point_i
{
    return {static_cast<i32>(mp.X / Scale->Width), static_cast<i32>(mp.Y / Scale->Height)};
}
}
