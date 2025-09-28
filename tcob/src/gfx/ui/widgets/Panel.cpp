// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/ui/widgets/Panel.hpp"

#include <algorithm>
#include <memory>
#include <optional>
#include <ranges>
#include <span>

#include "tcob/core/Point.hpp"
#include "tcob/core/Rect.hpp"
#include "tcob/core/ServiceLocator.hpp"
#include "tcob/core/input/Input.hpp"
#include "tcob/gfx/Transform.hpp"
#include "tcob/gfx/ui/Form.hpp"
#include "tcob/gfx/ui/Layout.hpp"
#include "tcob/gfx/ui/Style.hpp"
#include "tcob/gfx/ui/StyleElements.hpp"
#include "tcob/gfx/ui/UI.hpp"
#include "tcob/gfx/ui/WidgetPainter.hpp"
#include "tcob/gfx/ui/widgets/Widget.hpp"
#include "tcob/gfx/ui/widgets/WidgetContainer.hpp"

namespace tcob::ui {

void panel::style::Transition(style& target, style const& from, style const& to, f64 step)
{
    widget_style::Transition(target, from, to, step);

    target.HScrollBar.lerp(from.HScrollBar, to.HScrollBar, step);
    target.VScrollBar.lerp(from.VScrollBar, to.VScrollBar, step);
}

panel::panel(init const& wi)
    : widget_container {wi}
    , _layout {std::make_unique<panel::default_layout>(this)}
    , _vScrollbar {orientation::Vertical}
    , _hScrollbar {orientation::Horizontal}
{
    _vScrollbar.ValueChanged.connect([this] {
        form().rehover_widget(this);
        queue_redraw();
    });
    _hScrollbar.ValueChanged.connect([this] {
        form().rehover_widget(this);
        queue_redraw();
    });

    _layout->Changed.connect([&] { queue_redraw(); });

    ScrollEnabled.Changed.connect([this](auto const&) { queue_redraw(); });
    ScrollEnabled(false);

    Class("panel");
}

void panel::on_prepare_redraw()
{
    _layout->apply(content_bounds().Size);

    widget_container::on_prepare_redraw();
}

void panel::on_styles_changed()
{
    widget_container::on_styles_changed();

    _vScrollbar.reset(0);
    _hScrollbar.reset(0);
}

auto panel::requires_scroll(orientation orien, rect_f const& rect) const -> bool
{
    if (!ScrollEnabled) { return false; }

    return std::ranges::any_of(widgets(), [orien, rect](auto const& w) {
        auto const& bounds {*w->Bounds};
        if (orien == orientation::Horizontal) {
            if (bounds.left() < 0) { return true; }
            if (bounds.right() > rect.width()) { return true; }
        } else if (orien == orientation::Vertical) {
            if (bounds.top() < 0) { return true; }
            if (bounds.bottom() > rect.height()) { return true; }
        }

        return false;
    });
}

auto panel::widgets() const -> std::span<std::shared_ptr<widget> const>
{
    return _layout->widgets();
}

void panel::clear()
{
    _layout->clear();
}

void panel::on_update(milliseconds deltaTime)
{
    _vScrollbar.update(deltaTime);
    _hScrollbar.update(deltaTime);

    check_mode();
    if (_currentMode) {
        form().change_cursor_mode(*_currentMode);
    }
}

void panel::on_draw(widget_painter& painter)
{
    rect_f rect {draw_background(_style, painter)};

    // scrollbars
    {
        _vScrollbar.Visible = requires_scroll(orientation::Vertical, rect);
        auto const  vThumbFlags {!_vScrollbar.is_mouse_over_thumb() ? widget_flags {.Disabled = !is_enabled()} : flags()};
        thumb_style vThumbStyle;
        prepare_sub_style(vThumbStyle, -2, _style.VScrollBar.ThumbClass, vThumbFlags);
        _vScrollbar.draw(painter, _style.VScrollBar, vThumbStyle.Thumb, rect);
    }
    {
        _hScrollbar.Visible = requires_scroll(orientation::Horizontal, rect);
        auto const  hThumbFlags {!_hScrollbar.is_mouse_over_thumb() ? widget_flags {.Disabled = !is_enabled()} : flags()};
        thumb_style hThumbStyle;
        prepare_sub_style(hThumbStyle, -3, _style.HScrollBar.ThumbClass, hThumbFlags);
        _hScrollbar.draw(painter, _style.HScrollBar, hThumbStyle.Thumb, rect);
    }
}

void panel::on_draw_children(widget_painter& painter)
{
    prepare_style(_style);

    rect_f rect {content_bounds()};

    // content
    scoped_scissor const guard {painter, this};

    auto          xform {gfx::transform::Identity};
    point_f const translate {rect.Position + form_offset() - scroll_offset()};
    xform.translate(translate);

    for (auto const& w : widgets() | std::views::reverse) { // ZORDER
        painter.begin(Alpha, xform);
        w->draw(painter);
        painter.end();
    }
}

void panel::on_mouse_leave()
{
    _vScrollbar.mouse_leave();
    _hScrollbar.mouse_leave();
}

void panel::on_mouse_hover(input::mouse::motion_event const& ev)
{
    auto const scrollHover {[&](auto&& scrollbar) -> bool {
        return scrollbar.mouse_hover(*this, ev.Position);
    }};

    _overScrollBars = ev.Handled = scrollHover(_vScrollbar) || scrollHover(_hScrollbar);
}

void panel::on_mouse_drag(input::mouse::motion_event const& ev)
{
    auto const scrollDrag {[this, &ev](auto&& scrollbar) -> bool {
        return scrollbar.mouse_drag(*this, ev.Position);
    }};

    ev.Handled = scrollDrag(_vScrollbar) || scrollDrag(_hScrollbar);
    if (ev.Handled) { return; }

    if (_dragStart && _currentMode) {
        rect_f     newBounds {*Bounds};
        auto const localPos {screen_to_local(*this, ev.Position)};

        switch (*_currentMode) {
        case cursor_mode::Move: {
            newBounds = {localPos - *_dragStart, Bounds->Size};
            break;
        }
        case cursor_mode::W_Resize: {
            f32 const newX {localPos.X};
            f32 const deltaW {Bounds->left() - newX};
            newBounds.Position.X = newX;
            newBounds.Size.Width += deltaW;
            break;
        }
        case cursor_mode::E_Resize: {
            newBounds.Size.Width = localPos.X - Bounds->left();
            break;
        }
        case cursor_mode::N_Resize: {
            f32 const newY {localPos.Y};
            f32 const deltaH {Bounds->top() - newY};
            newBounds.Position.Y = newY;
            newBounds.Size.Height += deltaH;
            break;
        }
        case cursor_mode::S_Resize: {
            newBounds.Size.Height = localPos.Y - Bounds->top();
            break;
        }
        case cursor_mode::SE_Resize: {
            newBounds.Size.Width  = localPos.X - Bounds->left();
            newBounds.Size.Height = localPos.Y - Bounds->top();
            break;
        }
        case cursor_mode::NW_Resize: {
            f32 const newX {localPos.X};
            f32 const deltaW {Bounds->left() - newX};
            newBounds.Position.X = newX;
            newBounds.Size.Width += deltaW;

            f32 const newY {localPos.Y};
            f32 const deltaH {Bounds->top() - newY};
            newBounds.Position.Y = newY;
            newBounds.Size.Height += deltaH;
            break;
        }
        case cursor_mode::NE_Resize: {
            newBounds.Size.Width = localPos.X - Bounds->left();

            f32 const newY {localPos.Y};
            f32 const deltaH {Bounds->top() - newY};
            newBounds.Position.Y = newY;
            newBounds.Size.Height += deltaH;
            break;
        }
        case cursor_mode::SW_Resize: {
            f32 const newX {localPos.X};
            f32 const deltaW {Bounds->left() - newX};
            newBounds.Position.X = newX;
            newBounds.Size.Width += deltaW;
            newBounds.Size.Height = localPos.Y - Bounds->top();
            break;
        }
        default: break;
        }

        if (newBounds.width() >= MinSize->Width && newBounds.width() <= MaxSize->Width
            && newBounds.height() >= MinSize->Height && newBounds.height() <= MaxSize->Height) {
            Bounds     = newBounds;
            ev.Handled = true;
        }
    }
}

void panel::on_mouse_button_down(input::mouse::button_event const& ev)
{
    auto const mp {ev.Position};

    if (can_move() || can_resize()) {
        _dragStart = screen_to_local(*this, mp) - Bounds->Position;
        ev.Handled = true;
    }

    if (_vScrollbar.Visible || _hScrollbar.Visible) {
        if (ev.Button == controls().PrimaryMouseButton) {
            _vScrollbar.mouse_down(*this, mp);
            _hScrollbar.mouse_down(*this, mp);
            ev.Handled = true;
            return;
        }
    }
}

void panel::on_mouse_button_up(input::mouse::button_event const& ev)
{
    _dragStart = std::nullopt;

    if (_vScrollbar.Visible || _hScrollbar.Visible) {
        if (ev.Button == controls().PrimaryMouseButton) {
            _vScrollbar.mouse_up(*this, ev.Position);
            _hScrollbar.mouse_up(*this, ev.Position);
            ev.Handled = true;
            return;
        }
    }
}

void panel::on_mouse_wheel(input::mouse::wheel_event const& ev)
{
    if (_vScrollbar.Visible || _hScrollbar.Visible) {
        orientation orien {};
        bool        invert {};

        if (ev.Scroll.Y != 0) {
            orien  = orientation::Vertical;
            invert = ev.Scroll.Y > 0;
        } else if (ev.Scroll.X != 0) {
            orien  = orientation::Horizontal;
            invert = ev.Scroll.X < 0;
        }

        f32 const diff {invert ? -0.2f : 0.2f};
        if (orien == orientation::Vertical) {
            _vScrollbar.start(diff);
        } else if (orien == orientation::Horizontal) {
            _hScrollbar.start(diff);
        }

        ev.Handled = true;
    }
}

void panel::offset_content(rect_f& bounds, bool isHitTest) const
{
    widget::offset_content(bounds, isHitTest);

    // subtract scrollbars from content
    if (isHitTest) { return; }
    if (_vScrollbar.Visible) { bounds.Size.Width -= _style.VScrollBar.Bar.Size.calc(bounds.width()); }
    if (_hScrollbar.Visible) { bounds.Size.Height -= _style.HScrollBar.Bar.Size.calc(bounds.height()); }
}

auto panel::get_layout() const -> layout*
{
    return _layout.get();
}

auto panel::can_move() const -> bool
{
    return *Movable && !_overScrollBars && form().allows_move() && form().top_widget() == this;
}

auto panel::can_resize() const -> bool
{
    return *Resizable && !_overScrollBars && form().allows_resize() && form().top_widget() == this;
}

void panel::check_mode()
{
    if (_dragStart) { return; }
    auto const mp {locate_service<input::system>().mouse()->get_position()};

    _currentMode = can_move() ? std::optional {cursor_mode::Move} : std::nullopt;
    if (!can_resize()) { return; }

    auto const inBounds {hit_test_bounds()};
    auto const outBounds {inBounds - thickness {_style.Border.Size}};

    if (inBounds.contains(mp) && !outBounds.contains(mp)) {
        bool const onLeft {mp.X < static_cast<i32>(outBounds.left())};
        bool const onRight {mp.X > static_cast<i32>(outBounds.right())};

        bool const onTop {mp.Y < static_cast<i32>(outBounds.top())};
        bool const onBottom {mp.Y > static_cast<i32>(outBounds.bottom())};

        if (onLeft && onTop) {
            _currentMode = cursor_mode::NW_Resize;
        } else if (onRight && onBottom) {
            _currentMode = cursor_mode::SE_Resize;
        } else if (onRight && onTop) {
            _currentMode = cursor_mode::NE_Resize;
        } else if (onLeft && onBottom) {
            _currentMode = cursor_mode::SW_Resize;
        } else if (onLeft) {
            _currentMode = cursor_mode::W_Resize;
        } else if (onRight) {
            _currentMode = cursor_mode::E_Resize;
        } else if (onTop) {
            _currentMode = cursor_mode::N_Resize;
        } else if (onBottom) {
            _currentMode = cursor_mode::S_Resize;
        }
    }
}

auto panel::get_scroll_max_value(orientation orien) const -> f32
{
    f32         retValue {0.0f};
    auto const& content {content_bounds()};
    for (auto const& w : widgets()) {
        auto const& bounds {*w->Bounds};
        retValue = std::max(retValue,
                            orien == orientation::Horizontal
                                ? bounds.right() - content.width()
                                : bounds.bottom() - content.height());
    }
    return retValue * 1.05f;
}

auto panel::scroll_offset() const -> point_f
{
    if (!ScrollEnabled) { return point_f::Zero; }
    return {_hScrollbar.current_value() * get_scroll_max_value(orientation::Horizontal),
            _vScrollbar.current_value() * get_scroll_max_value(orientation::Vertical)};
}

////////////////////////////////////////////////////////////

glass::glass(init const& wi)
    : panel {wi}
{
    Class("glass");
}

void glass::on_draw(widget_painter& painter)
{
    if (is_top_level()) {
        auto& canvas {painter.canvas()};
        canvas.begin_path();
        canvas.rect(*Bounds);
        canvas.clear();
    }
}

void glass::on_draw_children(widget_painter& painter)
{
    rect_f rect {content_bounds()};

    // content
    scoped_scissor const guard {painter, this};

    auto          xform {gfx::transform::Identity};
    point_f const translate {rect.Position + form_offset() - scroll_offset()};
    xform.translate(translate);

    for (auto const& w : this->widgets() | std::views::reverse) { // ZORDER
        painter.begin(Alpha, xform);
        w->draw(painter);
        painter.end();
    }
}

auto glass::is_inert() const -> bool
{
    return true;
}

////////////////////////////////////////////////////////////

modal_dialog::modal_dialog(init const& wi)
    : panel {wi}
{
    Class("modal_dialog");
}

void modal_dialog::open()
{
    form().push_modal(this); // always push to top
    if (!_open) {
        _open = true;
        on_open();
    }
    queue_redraw();
}

void modal_dialog::close()
{
    if (_open) {
        form().pop_modal(this);
        _open = false;
        on_close();
        queue_redraw();
    }
}

auto modal_dialog::is_open() const -> bool
{
    return _open;
}

}
