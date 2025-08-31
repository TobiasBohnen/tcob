// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/ui/Form.hpp"

#include <algorithm>
#include <cassert>
#include <chrono>
#include <limits>
#include <memory>
#include <ranges>
#include <utility>
#include <vector>

#include "tcob/core/Common.hpp"
#include "tcob/core/Interfaces.hpp"
#include "tcob/core/Point.hpp"
#include "tcob/core/Property.hpp"
#include "tcob/core/Rect.hpp"
#include "tcob/core/ServiceLocator.hpp"
#include "tcob/core/Size.hpp"
#include "tcob/core/input/Input.hpp"
#include "tcob/gfx/RenderTarget.hpp"
#include "tcob/gfx/drawables/Drawable.hpp"
#include "tcob/gfx/ui/Style.hpp"
#include "tcob/gfx/ui/UI.hpp"
#include "tcob/gfx/ui/WidgetPainter.hpp"
#include "tcob/gfx/ui/widgets/Tooltip.hpp"
#include "tcob/gfx/ui/widgets/Widget.hpp"
#include "tcob/gfx/ui/widgets/WidgetContainer.hpp"

namespace tcob::ui {

using namespace std::chrono_literals;

////////////////////////////////////////////////////////////

form_base::form_base(string name, rect_f const& bounds)
    : entity {update_mode::Normal}
    , Bounds {bounds}
    , _renderer {_canvas}
    , _painter {std::make_unique<widget_painter>(_canvas)}
    , _name {std::move(name)}
{
    Bounds.Changed.connect([this](auto const&) { on_bounds_changed(); });
    _renderer.set_bounds(*Bounds);

    Styles.Changed.connect([this](auto const&) { on_styles_changed(); });

    Shader.Changed.connect([this](auto const& value) { _renderer.set_shader(value); });
}

form_base::~form_base()
{
    // TODO: disconnect ALL events
}

auto form_base::name() const -> string const&
{
    return _name;
}

auto form_base::top_widget() const -> widget*
{
    return _topWidget;
}

auto form_base::find_widget_at(point_i pos) const -> widget*
{
    auto* modal {active_modal()};

    auto const widgets {modal ? modal->widgets() : get_layout()->widgets()};
    for (auto const& widget : widgets) { // ZORDER
        if (!widget->hit_test(pos)) { continue; }
        if (auto container {std::dynamic_pointer_cast<widget_container>(widget)}) {
            if (auto retValue {container->find_child_at(pos)}) {
                return retValue.get();
            }
        }
        return widget.get();
    }

    return modal && modal->hit_test(pos) ? modal : nullptr;
}

auto form_base::find_widget_by_name(string const& name) const -> widget*
{
    for (auto const& widget : containers()) {
        if (widget->name() == name) { return widget.get(); }
        if (auto container {std::dynamic_pointer_cast<widget_container>(widget)}) {
            if (auto retValue {container->find_child_by_name(name)}) {
                return retValue.get();
            }
        }
    }
    return nullptr;
}

auto form_base::all_widgets() const -> std::vector<widget*>
{
    std::vector<widget*> retValue;

    auto const collectWidgets {[&retValue](this auto&& self, widget_container const& container) -> void {
        for (auto const& widget : container.widgets()) {
            retValue.push_back(widget.get());
            if (auto widgetContainer {std::dynamic_pointer_cast<widget_container>(widget)}) {
                self(*widgetContainer);
            }
        }
    }};

    if (auto* modal {active_modal()}) {
        retValue.push_back(modal);
        collectWidgets(*modal);
    } else {
        for (auto const& widget : containers()) {
            retValue.push_back(widget.get());
            if (auto container {std::dynamic_pointer_cast<widget_container>(widget)}) {
                collectWidgets(*container);
            }
        }
    }
    return retValue;
}

void form_base::queue_redraw()
{
    for (auto const& widget : containers()) {
        widget->set_redraw(true);
    }
    notify_redraw();
}

void form_base::notify_redraw()
{
    _prepareWidgets = true;
}

void form_base::push_modal(modal_dialog* dlg)
{
    auto it {std::ranges::find(_modals, dlg)};
    if (it != _modals.end()) { _modals.erase(it); }

    _modals.push_back(dlg);
    focus_widget(dlg);
    _topWidget = nullptr; // TODO: mouse left
}

void form_base::pop_modal(modal_dialog* dlg)
{
    auto it {std::ranges::find(_modals, dlg)};
    if (it != _modals.end()) {
        _modals.erase(it);
        focus_widget(active_modal());
        _topWidget = nullptr; // TODO: mouse left
    }
}

auto form_base::active_modal() const -> modal_dialog*
{
    return _modals.empty() ? nullptr : _modals.back();
}

void form_base::on_update(milliseconds deltaTime)
{
    auto const& widgets {containers()};

    // tooltip
    handle_tooltip(deltaTime);

    // update styles
    if (_prepareWidgets) {
        // layout
        apply_layout();

        // containers
        for (auto const& container : widgets) {
            container->prepare_redraw();
        }

        // tooltips
        std::erase_if(_tooltips, [](auto const& tooltip) { return tooltip.expired(); });
        for (auto const& tooltip : _tooltips) {
            tooltip.lock()->prepare_redraw();
        }

        // modal
        if (auto* modal {active_modal()}) {
            modal->prepare_redraw();
        }

        _prepareWidgets = false;
        _redrawWidgets  = true;
    }

    // update widgets
    for (auto const& container : widgets) {
        container->update(deltaTime);
    }

    // update modal
    if (auto* modal {active_modal()}) {
        modal->update(deltaTime);
    }
}

auto form_base::can_draw() const -> bool
{
    return true;
}

void form_base::on_draw_to(gfx::render_target& target)
{
    constexpr static i32 overlayLayer {0};
    constexpr static i32 tooltipLayer {1};
    constexpr static i32 modalLayer {2};
    constexpr static i32 firstUILayer {modalLayer + 1};

    // set cursor
    if (_topWidget) {
        CursorChanged(_topWidget->Cursor);
    }

    size_i const size {size_i {Bounds->Size}};

    auto const widgets {get_layout()->widgets()};
    auto const layerCount {static_cast<i32>(widgets.size())};

    // ui
    // redraw
    if (_redrawWidgets) {
        i32 i {0};
        for (auto const& container : widgets | std::views::reverse) { // ZORDER
            if (container->needs_redraw()) {
                _canvas.begin_frame(size, 1.0f, firstUILayer + i);
                container->draw(*_painter);
                _canvas.end_frame();
            }
            ++i;
        }

        _canvas.begin_frame(size, 1.0f, overlayLayer);
        _drawOverlay = _painter->draw_overlays();
        _canvas.end_frame();
        _redrawWidgets = false;
    }

    // render
    for (i32 j {0}; j < layerCount; ++j) {
        _renderer.set_layer(firstUILayer + j);
        _renderer.render_to_target(target);
    }

    // overlay
    if (_drawOverlay) {
        _renderer.set_layer(overlayLayer);
        _renderer.render_to_target(target);
    }

    // tooltip
    if (_isTooltipVisible && _topWidget && _topWidget->Tooltip) {
        auto ttBounds {*_topWidget->Tooltip->Bounds};
        ttBounds.Position = point_f {locate_service<input::system>().mouse()->get_position()} - Bounds->Position + TooltipOffset;
        if (ttBounds.right() > Bounds->right()) { ttBounds.Position.X -= ttBounds.width() + TooltipOffset.X; }
        if (ttBounds.bottom() > Bounds->bottom()) { ttBounds.Position.Y -= ttBounds.height() + TooltipOffset.Y; }
        _topWidget->Tooltip->Bounds = ttBounds;

        _canvas.begin_frame(size, 1.0f, tooltipLayer);
        _topWidget->Tooltip->set_redraw(true);
        _topWidget->Tooltip->draw(*_painter);
        _canvas.end_frame();

        _renderer.set_layer(tooltipLayer);
        _renderer.render_to_target(target);
    }

    // modal
    if (auto* modal {active_modal()}) {
        if (modal->needs_redraw()) {
            _canvas.begin_frame(size, 1.0f, modalLayer);
            for (auto* m : _modals) {
                m->set_redraw(true);
                m->draw(*_painter);
            }
            _canvas.end_frame();
        }
        _renderer.set_layer(modalLayer);
        _renderer.render_to_target(target);
    }
}

auto form_base::focused_widget() const -> widget*
{
    return _focusWidget;
}

void form_base::focus_widget(widget* newFocus)
{
    if (newFocus == _focusWidget) { return; }

    _currentTabIndex = -1;
    _injector.on_focus_lost(_focusWidget);

    _focusWidget = newFocus;

    if (_focusWidget) {
        if (_focusWidget->is_inert()) {
            _focusWidget = nullptr;
            return;
        }

        auto* layout {get_layout()};
        if (layout->allows_move()) {
            layout->bring_to_front(_focusWidget->top_level_widget());
        }

        _currentTabIndex = _focusWidget->TabStop->Index;
        _injector.on_focus_gained(_focusWidget);
    }
}

void form_base::rehover_widget(widget* widget)
{
    point_i const mp {locate_service<input::system>().mouse()->get_position()};
    if (!widget->hit_test(mp)) { return; }

    widget->prepare_redraw();
    on_mouse_hover({.Position = mp});
}

void form_base::on_key_down(input::keyboard::event const& ev)
{
    hide_tooltip();

    using namespace tcob::enum_ops;

    if (ev.KeyCode == Controls->TabKey) {
        handle_tab(ev);
    } else if (!ev.Keyboard->is_key_down(Controls->ActivateKey) && _focusWidget) {
        handle_nav(ev);
    } else if (ev.KeyMods.is_down(Controls->CutCopyPasteMod) && ev.KeyCode == Controls->PasteKey) {
        input::keyboard::text_input_event tev {.Text = locate_service<input::system>().clipboard()->get_text()};
        if (!tev.Text.empty()) {
            _injector.on_text_input(_focusWidget, tev);
            ev.Handled = true;
        }
    }

    if (!ev.Handled) {
        _injector.on_key_down(_focusWidget, ev);
    }
}

void form_base::on_key_up(input::keyboard::event const& ev)
{
    hide_tooltip();
    _injector.on_key_up(_focusWidget, ev);
}

void form_base::on_mouse_motion(input::mouse::motion_event const& ev)
{
    if (_isLButtonDown) { // FIXME: restict (widget::can_drag?) TODO: add drag origin
        _injector.on_mouse_drag(_focusWidget, ev);
    } else {
        on_mouse_hover(ev);
    }
}

void form_base::on_mouse_hover(input::mouse::motion_event const& ev)
{
    auto* newTop {find_widget_at(ev.Position)};

    if (newTop && newTop->is_inert()) { // inert top
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

void form_base::on_mouse_button_down(input::mouse::button_event const& ev)
{
    hide_tooltip();

    focus_widget(_topWidget);
    if (_topWidget) {
        _injector.on_mouse_button_down(_topWidget, ev);
        if (ev.Button == Controls->PrimaryMouseButton) {
            _isLButtonDown = true;
        } else if (ev.Button == Controls->SecondaryMouseButton) {
            _isRButtonDown = true;
        }
    }
}

void form_base::on_mouse_button_up(input::mouse::button_event const& ev)
{
    hide_tooltip();

    if (!_focusWidget) { return; }
    _injector.on_mouse_button_up(_focusWidget, ev);

    if (ev.Button == Controls->PrimaryMouseButton) {
        _isLButtonDown = false;

        if (_topWidget == _focusWidget) {
            _injector.on_click(_focusWidget);

            if (ev.Clicks == 1) {
                _clickPos = ev.Position;
            } else if (ev.Clicks == 2 && _clickPos.distance_to(ev.Position) <= 5) {
                _injector.on_double_click(_focusWidget);
            }

            ev.Handled = true;
        }

    } else if (ev.Button == Controls->SecondaryMouseButton) {
        _isRButtonDown = false;
    }
}

void form_base::on_mouse_wheel(input::mouse::wheel_event const& ev)
{
    hide_tooltip();

    if (_topWidget) {
        _injector.on_mouse_wheel(_topWidget, ev);
    } else if (_focusWidget) {
        _injector.on_mouse_wheel(_focusWidget, ev);
    }
}

void form_base::on_controller_axis_motion(input::controller::axis_event const& /*ev*/)
{
}

void form_base::on_controller_button_down(input::controller::button_event const& ev)
{
    hide_tooltip();
    if (!ev.Controller->is_button_pressed(Controls->ActivateButton) && _focusWidget) {
        if (ev.Button == Controls->NavLeftButton) {
            ev.Handled = focus_nav_target(_focusWidget->name(), direction::Left);
        } else if (ev.Button == Controls->NavRightButton) {
            ev.Handled = focus_nav_target(_focusWidget->name(), direction::Right);
        } else if (ev.Button == Controls->NavDownButton) {
            ev.Handled = focus_nav_target(_focusWidget->name(), direction::Down);
        } else if (ev.Button == Controls->NavUpButton) {
            ev.Handled = focus_nav_target(_focusWidget->name(), direction::Up);
        }
    } else {
        _injector.on_controller_button_down(_focusWidget, ev);
    }
}

void form_base::on_controller_button_up(input::controller::button_event const& ev)
{
    hide_tooltip();
    _injector.on_controller_button_up(_focusWidget, ev);
}

void form_base::on_bounds_changed()
{
    _renderer.set_bounds(*Bounds);

    queue_redraw();
    on_styles_changed();
}

void form_base::on_visiblity_changed()
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

void form_base::on_text_input(input::keyboard::text_input_event const& ev)
{
    _injector.on_text_input(_focusWidget, ev);
}

void form_base::handle_tab(input::keyboard::event const& ev)
{
    auto static find_next_tab_widget {[](std::vector<widget*> const& vec, i32 currentTabIndex) -> widget* {
        widget* retValue {nullptr};
        i32     lowestHigherValue {std::numeric_limits<i32>::max()};
        for (auto* widget : vec) {
            if (widget->can_tab_stop(lowestHigherValue, currentTabIndex)) {
                lowestHigherValue = widget->TabStop->Index;
                retValue          = widget;
            }
        }
        return retValue;
    }};

    auto static find_prev_tab_widget {[](std::vector<widget*> const& vec, i32 currentTabIndex) -> widget* {
        widget* retValue {nullptr};
        i32     highestLowerValue {std::numeric_limits<i32>::min()};
        for (auto* widget : vec) {
            if (widget->can_tab_stop(currentTabIndex, highestLowerValue)) {
                highestLowerValue = widget->TabStop->Index;
                retValue          = widget;
            }
        }
        return retValue;
    }};

    auto const vec {all_widgets()};

    ev.Handled = true;

    if (ev.KeyMods.is_down(Controls->TabMod)) {
        // shift tab
        widget* nextWidget {find_prev_tab_widget(vec, _currentTabIndex)};
        if (!nextWidget) {
            _currentTabIndex = std::numeric_limits<i32>::max();
            nextWidget       = find_prev_tab_widget(vec, _currentTabIndex);
        }
        focus_widget(nextWidget);
    } else {
        // tab
        widget* nextWidget {find_next_tab_widget(vec, _currentTabIndex)};
        if (!nextWidget) {
            _currentTabIndex = -1;
            nextWidget       = find_next_tab_widget(vec, _currentTabIndex);
        }
        focus_widget(nextWidget);
    }
}

void form_base::handle_nav(input::keyboard::event const& ev)
{
    if (ev.KeyCode == Controls->NavLeftKey) {
        ev.Handled = focus_nav_target(_focusWidget->name(), direction::Left);
    } else if (ev.KeyCode == Controls->NavRightKey) {
        ev.Handled = focus_nav_target(_focusWidget->name(), direction::Right);
    } else if (ev.KeyCode == Controls->NavDownKey) {
        ev.Handled = focus_nav_target(_focusWidget->name(), direction::Down);
    } else if (ev.KeyCode == Controls->NavUpKey) {
        ev.Handled = focus_nav_target(_focusWidget->name(), direction::Up);
    }
}

auto form_base::focus_nav_target(string const& widget, direction dir) -> bool
{
    if (!NavMap->contains(widget)) { return false; }

    string navTarget;
    switch (dir) {
    case direction::Left:  navTarget = NavMap->at(widget).Left; break;
    case direction::Right: navTarget = NavMap->at(widget).Right; break;
    case direction::Up:    navTarget = NavMap->at(widget).Up; break;
    case direction::Down:  navTarget = NavMap->at(widget).Down; break;
    case direction::None:  break;
    }

    if (!navTarget.empty()) {
        if (auto* target {find_widget_by_name(navTarget)}) {
            focus_widget(target);
            return true;
        }
    }

    return false;
}

void form_base::on_styles_changed()
{
    _prepareWidgets = true;
    for (auto const& container : containers()) {
        container->on_styles_changed();
    }

    for (auto const& tt : _tooltips) {
        if (tt.expired()) { continue; }
        auto tooltip {tt.lock()};

        widget_style_selectors const ttNewSelectors {
            .Class      = tooltip->Class,
            .Flags      = tooltip->flags(),
            .Attributes = tooltip->attributes(),
        };
        auto* style {dynamic_cast<widget_style*>(Styles->get(ttNewSelectors))};
        assert(style);
        tooltip->_transition.reset(style);
        tooltip->on_styles_changed();
    }
}

void form_base::handle_tooltip(milliseconds deltaTime)
{
    if (!_topWidget) { return; }

    _mouseOverTime += deltaTime;

    auto const shouldtooltip {[this] {
        if (_isTooltipVisible) { return false; }
        if (locate_service<input::system>().InputMode != input::mode::KeyboardMouse) { return false; }

        bool const hasTooltip {_topWidget && _topWidget->Tooltip};
        bool const isTopFocused {_topWidget && _topWidget->is_focused()};
        bool const isMouseButtonDown {_isLButtonDown || _isRButtonDown};

        if (hasTooltip && !isTopFocused && !isMouseButtonDown) {
            return _mouseOverTime > _topWidget->Tooltip->Delay;
        }

        return false;
    }};
    if (shouldtooltip()) {
        _topWidget->Tooltip->on_tooltip(_topWidget);
        _isTooltipVisible = true;
    }

    if (_isTooltipVisible) {
        _topWidget->Tooltip->update(deltaTime);
    }
}

void form_base::hide_tooltip()
{
    _mouseOverTime    = 0ms;
    _isTooltipVisible = false;
}
}
