// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include <algorithm>

#include "tcob/gfx/ui/widgets/Panel.hpp"

#include "tcob/gfx/ui/Form.hpp"
#include "tcob/gfx/ui/Layout.hpp"
#include "tcob/gfx/ui/WidgetPainter.hpp"

namespace tcob::gfx::ui {
panel::panel(init const& wi)
    : widget_container {wi}
    , _layout {std::make_shared<fixed_layout>(this)}
    , _vScrollbar {*this, orientation::Vertical}
    , _hScrollbar {*this, orientation::Horizontal}
{
    ScrollEnabled.Changed.connect([&](auto const&) { force_redraw(this->name() + ": ScrollEnabled changed"); });
    ScrollEnabled(false);

    Class("panel");
}

auto panel::get_scroll_min_value(orientation orien) const -> f32
{
    f32 retValue {std::numeric_limits<f32>::max()};
    for (auto const& w : widgets()) {
        auto const& bounds {w->Bounds()};
        if (orien == orientation::Horizontal) {
            retValue = std::min(retValue, bounds.left());
        } else if (orien == orientation::Vertical) {
            retValue = std::min(retValue, bounds.top());
        }
    }
    retValue = std::min<f32>(retValue, 0);

    return retValue * 1.05f;
}

auto panel::get_scroll_max_value(orientation orien) const -> f32
{
    f32 retValue {0.0f};
    for (auto const& w : widgets()) {
        auto const& bounds {w->Bounds()};
        auto const  content {content_bounds()};
        if (orien == orientation::Horizontal) {
            retValue = std::max(retValue, bounds.right() - content.width());
        } else if (orien == orientation::Vertical) {
            retValue = std::max(retValue, bounds.bottom() - content.height());
        }
    }

    return retValue * 1.05f;
}

auto panel::get_scroll_style(orientation orien) const -> element::scrollbar*
{
    if (auto* style {current_style<panel::style>()}) {
        if (orien == orientation::Horizontal) {
            return &style->HScrollBar;
        }
        if (orien == orientation::Vertical) {
            return &style->VScrollBar;
        }
    }

    return nullptr;
}

void panel::force_redraw(string const& reason)
{
    widget_container::force_redraw(reason);
    _layout->mark_dirty();
}

void panel::on_styles_changed()
{
    widget_container::on_styles_changed();
    _layout->mark_dirty();
    _vScrollbar.set_target_value(0, milliseconds {0});
    _hScrollbar.set_target_value(0, milliseconds {0});
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

void panel::on_paint(widget_painter& painter)
{
    if (auto const* style {current_style<panel::style>()}) {
        rect_f rect {Bounds()};

        // background
        painter.draw_background_and_border(*style, rect, false);

        // scrollbar
        _vScrollbar.paint(painter, style->VScrollBar, rect, get_flags().Active);
        _hScrollbar.paint(painter, style->HScrollBar, rect, get_flags().Active);

        // content
        scissor_guard const guard {painter, this};

        auto          xform {transform::Identity};
        point_f const translate {rect.Position + get_paint_offset()};
        xform.translate(translate);

        for (auto const& w : widgets_by_zorder()) {
            painter.begin(Alpha(), xform);
            w->paint(painter);
            painter.end();
        }
    }
}

void panel::on_update(milliseconds deltaTime)
{
    _layout->update();

    _vScrollbar.update(deltaTime);
    _hScrollbar.update(deltaTime);
}

void panel::on_mouse_hover(input::mouse::motion_event const& ev)
{
    widget_container::on_mouse_hover(ev);

    if (_vScrollbar.Visible || _hScrollbar.Visible) {
        if (_vScrollbar.inject_mouse_hover(ev.Position)) {
            force_redraw(this->name() + ": vertical scrollbar hover");
            ev.Handled = true;
        }

        if (_hScrollbar.inject_mouse_hover(ev.Position)) {
            force_redraw(this->name() + ": horizontal scrollbar hover");
            ev.Handled = true;
        }
    }
}

void panel::on_mouse_drag(input::mouse::motion_event const& ev)
{
    widget_container::on_mouse_drag(ev);

    if (_vScrollbar.Visible || _hScrollbar.Visible) {
        if (_vScrollbar.inject_mouse_drag(ev.Position)) {
            force_redraw(this->name() + ": vertical scrollbar dragged");
            ev.Handled = true;
        }

        if (_hScrollbar.inject_mouse_drag(ev.Position)) {
            force_redraw(this->name() + ": horizontal scrollbar dragged");
            ev.Handled = true;
        }
    }
}

void panel::on_mouse_down(input::mouse::button_event const& ev)
{
    widget_container::on_mouse_down(ev);

    if (_vScrollbar.Visible || _hScrollbar.Visible) {
        if (ev.Button == parent_form()->Controls->PrimaryMouseButton) {
            _vScrollbar.inject_mouse_down(ev.Position);
            _hScrollbar.inject_mouse_down(ev.Position);
            force_redraw(this->name() + ": mouse down");
            ev.Handled = true;
        }
    }
}

void panel::on_mouse_up(input::mouse::button_event const& ev)
{
    widget_container::on_mouse_up(ev);

    if (_vScrollbar.Visible || _hScrollbar.Visible) {
        if (ev.Button == parent_form()->Controls->PrimaryMouseButton) {
            _vScrollbar.inject_mouse_up(ev.Position);
            _hScrollbar.inject_mouse_up(ev.Position);
            force_redraw(this->name() + ": mouse up");
            ev.Handled = true;
        }
    }
}

void panel::on_mouse_wheel(input::mouse::wheel_event const& ev)
{
    widget_container::on_mouse_wheel(ev);

    if (_vScrollbar.Visible || _hScrollbar.Visible) {
        if (auto const* style {current_style<panel::style>()}) {
            orientation  orien {};
            bool         invert {false};
            milliseconds delay {};

            if (ev.Scroll.Y != 0) {
                orien  = orientation::Vertical;
                invert = ev.Scroll.Y > 0;
                delay  = style->VScrollBar.Bar.Delay;
            } else if (ev.Scroll.X != 0) {
                orien  = orientation::Horizontal;
                invert = ev.Scroll.X < 0;
                delay  = style->HScrollBar.Bar.Delay;
            }

            f32 const min {get_scroll_min_value(orien)};
            f32 const max {get_scroll_max_value(orien)};
            f32 const diff {(max - min) / (invert ? -5 : 5)};
            if (orien == orientation::Vertical) {
                _vScrollbar.set_target_value(_vScrollbar.get_target_value() + diff, delay);
            } else if (orien == orientation::Horizontal) {
                _hScrollbar.set_target_value(_hScrollbar.get_target_value() + diff, delay);
            }

            ev.Handled = true;
        }
    }
}

void panel::offset_content(rect_f& bounds, bool isHitTest) const
{
    widget::offset_content(bounds, isHitTest);

    if (!isHitTest) {
        if (auto const* style {current_style<panel::style>()}) {
            if (_vScrollbar.Visible) {
                bounds.Size.Width -= style->VScrollBar.Bar.Size.calc(bounds.width());
            }
            if (_hScrollbar.Visible) {
                bounds.Size.Height -= style->HScrollBar.Bar.Size.calc(bounds.height());
            }
        }
    }
}

auto panel::get_layout() -> std::shared_ptr<layout>
{
    return _layout;
}

auto panel::scroll_offset() const -> point_f
{
    return {_hScrollbar.get_current_value(), _vScrollbar.get_current_value()};
}

void panel::set_scroll_offset(point_f off)
{
    _hScrollbar.set_target_value(off.X, milliseconds {0});
    _vScrollbar.set_target_value(off.Y, milliseconds {0});
    force_redraw(this->name() + ": set_scroll_offset");
}

////////////////////////////////////////////////////////////

glass::glass(init const& wi)
    : panel {wi}
{
    Class("glass");
    set_inert(true);
}

void glass::on_paint(widget_painter& painter)
{
    rect_f rect {Bounds()};

    // content
    scissor_guard const guard {painter, this};

    auto          xform {transform::Identity};
    point_f const translate {rect.Position + get_paint_offset()};
    xform.translate(translate);

    for (auto const& w : widgets()) {
        painter.begin(Alpha(), xform);
        w->paint(painter);
        painter.end();
    }
}

} // namespace ui
