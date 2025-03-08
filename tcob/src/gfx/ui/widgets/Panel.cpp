// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/ui/widgets/Panel.hpp"

#include <algorithm>
#include <memory>
#include <vector>

#include "tcob/core/Point.hpp"
#include "tcob/core/Rect.hpp"
#include "tcob/core/input/Input.hpp"
#include "tcob/gfx/Gfx.hpp"
#include "tcob/gfx/Transform.hpp"
#include "tcob/gfx/ui/Form.hpp"
#include "tcob/gfx/ui/Layout.hpp"
#include "tcob/gfx/ui/Style.hpp"
#include "tcob/gfx/ui/UI.hpp"
#include "tcob/gfx/ui/WidgetPainter.hpp"
#include "tcob/gfx/ui/widgets/Widget.hpp"
#include "tcob/gfx/ui/widgets/WidgetContainer.hpp"

namespace tcob::gfx::ui {

void panel::style::Transition(style& target, style const& left, style const& right, f64 step)
{
    widget_style::Transition(target, left, right, step);

    element::scrollbar::Transition(target.HScrollBar, left.HScrollBar, right.HScrollBar, step);
    element::scrollbar::Transition(target.VScrollBar, left.VScrollBar, right.VScrollBar, step);
}

panel::panel(init const& wi)
    : widget_container {wi}
    , _layout {std::make_shared<fixed_layout>(this)}
    , _vScrollbar {*this, orientation::Vertical}
    , _hScrollbar {*this, orientation::Horizontal}
{
    ScrollEnabled.Changed.connect([this](auto const&) { force_redraw(this->name() + ": ScrollEnabled changed"); });
    ScrollEnabled(false);

    Class("panel");
}

void panel::prepare_redraw()
{
    widget_container::prepare_redraw();
    _layout->mark_dirty();
}

void panel::on_styles_changed()
{
    widget_container::on_styles_changed();
    _layout->mark_dirty();

    _vScrollbar.reset();
    _hScrollbar.reset();
}

auto panel::requires_scroll(orientation orien, rect_f const& rect) const -> bool
{
    if (!ScrollEnabled()) { return false; }

    return std::ranges::any_of(widgets(), [orien, rect](auto const& w) {
        auto const& bounds {w->Bounds()};
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

auto panel::widgets() const -> std::vector<std::shared_ptr<widget>> const&
{
    return _layout->widgets();
}

void panel::clear_widgets()
{
    _layout->clear();
    force_redraw(this->name() + ": clearing");
}

void panel::on_update(milliseconds deltaTime)
{
    // TODO: update style here
    _layout->update();

    _vScrollbar.update(deltaTime);
    _hScrollbar.update(deltaTime);
}

void panel::on_paint(widget_painter& painter)
{
    update_style(_style);

    rect_f rect {Bounds()};

    // background
    painter.draw_background_and_border(_style, rect, false);

    // scrollbars
    {
        _vScrollbar.Visible = requires_scroll(orientation::Vertical, rect);
        auto const  vThumbFlags {!_vScrollbar.is_mouse_over_thumb() ? widget_flags {}
                                     : flags().Active               ? widget_flags {.Active = true}
                                                                    : widget_flags {.Hover = true}};
        thumb_style vThumbStyle;
        update_sub_style(vThumbStyle, -2, _style.VScrollBar.ThumbClass, vThumbFlags);
        _vScrollbar.paint(painter, _style.VScrollBar, vThumbStyle.Thumb, rect);
    }
    {
        _hScrollbar.Visible = requires_scroll(orientation::Horizontal, rect);
        auto const  hThumbFlags {!_hScrollbar.is_mouse_over_thumb() ? widget_flags {}
                                     : flags().Active               ? widget_flags {.Active = true}
                                                                    : widget_flags {.Hover = true}};
        thumb_style hThumbStyle;
        update_sub_style(hThumbStyle, -3, _style.HScrollBar.ThumbClass, hThumbFlags);
        _hScrollbar.paint(painter, _style.HScrollBar, hThumbStyle.Thumb, rect);
    }

    // content
    scissor_guard const guard {painter, this};

    auto          xform {transform::Identity};
    point_f const translate {rect.Position + paint_offset()};
    xform.translate(translate);

    for (auto const& w : widgets_by_zorder(false)) {
        painter.begin(Alpha(), xform);
        w->paint(painter);
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
    auto const scrollHover {[&](auto&& scrollbar) {
        scrollbar.mouse_hover(ev.Position);
        if (scrollbar.is_mouse_over()) {
            force_redraw(this->name() + ": scrollbar hover");
            return true;
        }
        return false;
    }};

    ev.Handled = scrollHover(_vScrollbar) || scrollHover(_hScrollbar);
}

void panel::on_mouse_drag(input::mouse::motion_event const& ev)
{
    auto const scrollDrag {[&](auto&& scrollbar) {
        scrollbar.mouse_drag(ev.Position);
        if (scrollbar.is_dragging()) {
            force_redraw(this->name() + ": scrollbar dragged");
            return true;
        }
        return false;
    }};

    ev.Handled = scrollDrag(_vScrollbar) || scrollDrag(_hScrollbar);
}

void panel::on_mouse_down(input::mouse::button_event const& ev)
{
    if (_vScrollbar.Visible || _hScrollbar.Visible) {
        if (ev.Button == parent_form()->Controls->PrimaryMouseButton) {
            _vScrollbar.mouse_down(ev.Position);
            _hScrollbar.mouse_down(ev.Position);
            force_redraw(this->name() + ": mouse down");
            ev.Handled = true;
        }
    }
}

void panel::on_mouse_up(input::mouse::button_event const& ev)
{
    if (_vScrollbar.Visible || _hScrollbar.Visible) {
        if (ev.Button == parent_form()->Controls->PrimaryMouseButton) {
            _vScrollbar.mouse_up(ev.Position);
            _hScrollbar.mouse_up(ev.Position);
            force_redraw(this->name() + ": mouse up");
            ev.Handled = true;
        }
    }
}

void panel::on_mouse_wheel(input::mouse::wheel_event const& ev)
{
    if (_vScrollbar.Visible || _hScrollbar.Visible) {
        orientation  orien {};
        bool         invert {};
        milliseconds delay {};

        if (ev.Scroll.Y != 0) {
            orien  = orientation::Vertical;
            invert = ev.Scroll.Y > 0;
            delay  = _style.VScrollBar.Bar.Delay;
        } else if (ev.Scroll.X != 0) {
            orien  = orientation::Horizontal;
            invert = ev.Scroll.X < 0;
            delay  = _style.HScrollBar.Bar.Delay;
        }

        f32 const diff {invert ? -0.2f : 0.2f};
        if (orien == orientation::Vertical) {
            _vScrollbar.start_scroll(_vScrollbar.target_value() + diff, delay);
        } else if (orien == orientation::Horizontal) {
            _hScrollbar.start_scroll(_hScrollbar.target_value() + diff, delay);
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

auto panel::get_layout() -> std::shared_ptr<layout>
{
    return _layout;
}

auto panel::get_scroll_max_value(orientation orien) const -> f32
{
    f32         retValue {0.0f};
    auto const& content {content_bounds()};
    for (auto const& w : widgets()) {
        auto const& bounds {w->Bounds()};
        retValue = std::max(retValue, orien == orientation::Horizontal ? bounds.right() - content.width() : bounds.bottom() - content.height());
    }
    return retValue * 1.05f;
}

auto panel::scroll_offset() const -> point_f
{
    if (!ScrollEnabled) { return point_f::Zero; }
    return {_hScrollbar.current_value() * get_scroll_max_value(orientation::Horizontal), _vScrollbar.current_value() * get_scroll_max_value(orientation::Horizontal)};
}

////////////////////////////////////////////////////////////

glass::glass(init const& wi)
    : panel {wi}
{
    Class("glass");
}

void glass::on_paint(widget_painter& painter)
{
    rect_f rect {Bounds()};

    // content
    scissor_guard const guard {painter, this};

    auto          xform {transform::Identity};
    point_f const translate {rect.Position + paint_offset()};
    xform.translate(translate);

    for (auto const& w : widgets()) {
        painter.begin(Alpha(), xform);
        w->paint(painter);
        painter.end();
    }
}

auto glass::is_inert() const -> bool
{
    return true;
}

} // namespace ui
