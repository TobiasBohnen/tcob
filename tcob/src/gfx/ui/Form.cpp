// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/ui/Form.hpp"

#include <utility>

#include "tcob/core/Logger.hpp"
#include "tcob/core/ServiceLocator.hpp"
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
    : entity {update_mode::Fixed}
    , Bounds {bounds}
    , _renderer {_canvas}
    , _window {window}
    , _layout {this}
    , _painter {std::make_unique<widget_painter>(_canvas)}
    , _name {std::move(name)}
{
    Bounds.Changed.connect([this](auto const&) { on_bounds_changed(); });
    on_bounds_changed();

    Styles.Changed.connect([this](auto const&) { on_styles_changed(); });
}

form::~form()
{
    // TODO: disconnect ALL events
}

auto form::name() const -> string const&
{
    return _name;
}

void form::remove_container(widget* wc)
{
    _layout.remove_widget(wc);
}

auto form::top_widget() const -> widget*
{
    return _topWidget;
}

auto form::find_widget_at(point_f pos) const -> std::shared_ptr<widget>
{
    for (auto const& widget : widgets_by_zorder(true)) {
        if (!widget->hit_test(pos)) { continue; }
        if (auto container {std::dynamic_pointer_cast<widget_container>(widget)}) {
            if (auto retValue {container->find_child_at(pos)}) {
                return retValue;
            }
        }
        return widget;
    }
    return nullptr;
}

auto form::find_widget_by_name(string const& name) const -> std::shared_ptr<widget>
{
    for (auto const& widget : containers()) {
        if (widget->name() == name) { return widget; }
        if (auto container {std::dynamic_pointer_cast<widget_container>(widget)}) {
            if (auto retValue {container->find_child_by_name(name)}) {
                return retValue;
            }
        }
    }
    return nullptr;
}

auto form::containers() const -> std::vector<std::shared_ptr<widget>> const&
{
    return _layout.widgets();
}

auto form::all_widgets() const -> std::vector<widget*>
{
    auto const collectWidgets {[](std::vector<widget*>& vec, std::shared_ptr<widget_container> const& widget, auto&& collect) -> void {
        for (auto const& widget : widget->widgets()) {
            vec.push_back(widget.get());
            if (auto container {std::dynamic_pointer_cast<widget_container>(widget)}) {
                collect(vec, container, collect);
            }
        }
    }};

    std::vector<widget*> retValue;
    for (auto const& widget : containers()) {
        retValue.push_back(widget.get());
        if (auto container {std::dynamic_pointer_cast<widget_container>(widget)}) {
            collectWidgets(retValue, container, collectWidgets);
        }
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
    _updateWidgetStyle = true;
    _layout.mark_dirty();
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

void form::on_fixed_update(milliseconds deltaTime)
{
    auto const& widgets {containers()};

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
    if (_updateWidgetStyle) {
        for (auto const& container : widgets) {
            container->prepare_redraw();
        }

        _updateWidgetStyle = false;
        _redrawWidgets     = true;
    }

    // layout
    _layout.update();

    // update widgets
    for (auto const& container : widgets) {
        container->update(deltaTime);
    }
}

void form::on_styles_changed()
{
    _updateWidgetStyle = true;
    for (auto const& container : containers()) {
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
        _window->Cursor->ActiveMode = _topWidget->current_style<widget_style>()->Cursor;
    }

    size_i const bounds {size_i {Bounds->Size}};

    // redraw
    if (_redrawWidgets) {
        _canvas.begin_frame(bounds, 1.0f, 0);

        for (auto const& container : widgets_by_zorder()) {
            _canvas.reset();
            container->paint(*_painter);
        }

        _canvas.end_frame();
        _redrawWidgets = false;
    }

    // tooltip
    if (_isTooltipVisible && _topWidget && _topWidget->Tooltip) {
        point_i const pos {(_window && _window->Cursor()
                                ? _window->Cursor->bounds().bottom_right()
                                : locate_service<input::system>().mouse().get_position())
                           - static_cast<point_i>(Bounds->Position)};

        auto& ttBounds {*_topWidget->Tooltip->Bounds};
        ttBounds.Position.X = static_cast<f32>(pos.X);
        ttBounds.Position.Y = static_cast<f32>(pos.Y);
        if (ttBounds.right() > Bounds->right()) {
            ttBounds.Position.X -= ttBounds.width();
            if (_window && _window->Cursor()) {
                ttBounds.Position.X -= _window->Cursor->bounds().width();
            }
        }
        if (ttBounds.bottom() > Bounds->bottom()) {
            ttBounds.Position.Y -= ttBounds.height();
            if (_window && _window->Cursor()) {
                ttBounds.Position.Y -= _window->Cursor->bounds().height();
            }
        }

        _canvas.begin_frame(bounds, 1.0f, 1);
        _topWidget->Tooltip->paint(*_painter);
        _canvas.end_frame();
    }

    // render
    _renderer.set_layer(0);
    _renderer.render_to_target(target);

    if (_isTooltipVisible) {
        _renderer.set_layer(1);
        _renderer.render_to_target(target);
    }
}

auto form::focused_widget() const -> widget*
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

void form::find_top_widget(input::mouse::motion_event const& ev)
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
    } else {
        _injector.on_mouse_hover(_topWidget, ev);
    }
}

void form::on_key_down(input::keyboard::event const& ev)
{
    hide_tooltip();

    using namespace tcob::enum_ops;

    if (ev.KeyCode == Controls->TabKey) {
        auto const vec {all_widgets()};

        if ((ev.KeyMods & Controls->TabMod) == Controls->TabMod) {
            // shift tab
            widget* nextWidget {find_prev_tab_widget(vec)};
            if (!nextWidget) {
                _currentTabIndex = std::numeric_limits<i32>::max();
                nextWidget       = find_prev_tab_widget(vec);
            }
            focus_widget(nextWidget);
        } else {
            // tab
            widget* nextWidget {find_next_tab_widget(vec)};
            if (!nextWidget) {
                _currentTabIndex = -1;
                nextWidget       = find_next_tab_widget(vec);
            }
            focus_widget(nextWidget);
        }
    } else if ((ev.KeyMods & Controls->CutCopyPasteMod) == Controls->CutCopyPasteMod) {
        if (ev.KeyCode == Controls->PasteKey) {
            input::keyboard::text_input_event tev {.Text = locate_service<input::system>().clipboard().get_text()};
            _injector.on_text_input(_focusWidget, tev);
        }
    } else {
        _injector.on_key_down(_focusWidget, ev);
    }
}

void form::on_key_up(input::keyboard::event const& ev)
{
    hide_tooltip();
    _injector.on_key_up(_focusWidget, ev);
}

void form::on_mouse_motion(input::mouse::motion_event const& ev)
{
    if (_isLButtonDown) {
        _injector.on_mouse_drag(_focusWidget, ev);
    } else {
        find_top_widget(ev);
    }
}

void form::on_mouse_button_down(input::mouse::button_event const& ev)
{
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
}

void form::on_mouse_button_up(input::mouse::button_event const& ev)
{
    hide_tooltip();

    if (_focusWidget) {
        _injector.on_mouse_up(_focusWidget, ev);

        if (_topWidget == _focusWidget) {
            if (ev.Button == Controls->PrimaryMouseButton) {
                _injector.on_click(_focusWidget);

                if (ev.Clicks == 1) {
                    _clickPos = ev.Position;
                }
                if (ev.Clicks == 2 && _clickPos.distance_to(ev.Position) <= 5) {
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
}

void form::on_mouse_wheel(input::mouse::wheel_event const& ev)
{
    hide_tooltip();

    if (_topWidget) {
        _injector.on_mouse_wheel(_topWidget, ev);
    } else if (_focusWidget) {
        _injector.on_mouse_wheel(_focusWidget, ev);
    }
}

void form::on_controller_axis_motion(input::controller::axis_event const& /*ev*/)
{
}

void form::on_controller_button_down(input::controller::button_event const& ev)
{
    hide_tooltip();
    _injector.on_controller_button_down(_focusWidget, ev);
}

void form::on_controller_button_up(input::controller::button_event const& ev)
{
    hide_tooltip();
    _injector.on_controller_button_up(_focusWidget, ev);
}

void form::on_bounds_changed()
{
    _renderer.set_bounds(Bounds());

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

auto form::widgets_by_zorder(bool reverse) const -> std::vector<std::shared_ptr<widget>>
{
    auto retValue {containers()};
    if (reverse) {
        std::stable_sort(retValue.begin(), retValue.end(), [](auto const& a, auto const& b) { return a->ZOrder() > b->ZOrder(); });
    } else {
        std::stable_sort(retValue.begin(), retValue.end(), [](auto const& a, auto const& b) { return a->ZOrder() < b->ZOrder(); });
    }
    return retValue;
}

void form::on_text_input(input::keyboard::text_input_event const& ev)
{
    _injector.on_text_input(_focusWidget, ev);
}

void form::on_text_editing(input::keyboard::text_editing_event const& ev)
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
        && _topWidget && _topWidget->Tooltip
        && _focusWidget != _topWidget
        && !_isLButtonDown && !_isRButtonDown
        && locate_service<input::system>().InputMode == input::mode::KeyboardMouse) {

        if (auto* style {_topWidget->Tooltip->current_style<tooltip::style>()}) {
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

}
