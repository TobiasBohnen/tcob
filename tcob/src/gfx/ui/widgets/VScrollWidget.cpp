// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/ui/widgets/VScrollWidget.hpp"

#include "tcob/gfx/ui/Form.hpp"
#include "tcob/gfx/ui/WidgetPainter.hpp"

namespace tcob::gfx::ui {

vscroll_widget::vscroll_widget(init const& wi)
    : widget {wi}
    , _vScrollbar {*this, orientation::Vertical}
{
}

auto vscroll_widget::requires_scroll(orientation orien, rect_f const& rect) const -> bool
{
    if (orien == orientation::Horizontal) {
        return false;
    }

    return get_scroll_content_height() - 1 > rect.height();
}

auto vscroll_widget::get_scroll_min_value(orientation /* orien */) const -> f32
{
    return 0;
}

auto vscroll_widget::get_scroll_max_value(orientation orien) const -> f32
{
    if (orien == orientation::Horizontal) {
        return 0;
    }

    return std::max(0.0f, get_scroll_content_height() - content_bounds().height());
}

auto vscroll_widget::get_scroll_style(orientation orien) const -> element::scrollbar*
{
    if (auto* style {current_style<vscroll_widget::style>()}) {
        if (orien == orientation::Vertical) {
            return &style->VScrollBar;
        }
    }

    return nullptr;
}

void vscroll_widget::on_styles_changed()
{
    widget::on_styles_changed();
    f32 const max {get_scroll_max_value(orientation::Vertical)};
    if (get_scrollbar_value() > max) { set_scrollbar_value(max); }
}

void vscroll_widget::on_paint(widget_painter& painter)
{
    if (auto const* style {current_style<vscroll_widget::style>()}) {
        rect_f rect {Bounds()};

        // background
        painter.draw_background_and_border(*style, rect, false);

        // scrollbar
        _vScrollbar.paint(painter, style->VScrollBar, rect, flags().Active);

        // content
        scissor_guard const guard {painter, this};
        paint_content(painter, rect);
    }
}

void vscroll_widget::on_mouse_hover(input::mouse::motion_event const& ev)
{
    widget::on_mouse_hover(ev);

    if (_vScrollbar.inject_mouse_hover(ev.Position)) {
        force_redraw(this->name() + ": scrollbar mouse hover");
        ev.Handled = true;
    }
}

void vscroll_widget::on_mouse_down(input::mouse::button_event const& ev)
{
    widget::on_mouse_down(ev);

    if (ev.Button == parent_form()->Controls->PrimaryMouseButton) {
        _vScrollbar.inject_mouse_down(ev.Position);
        force_redraw(this->name() + ": mouse down");
        ev.Handled = true;
    }
}

void vscroll_widget::on_mouse_drag(input::mouse::motion_event const& ev)
{
    widget::on_mouse_drag(ev);

    if (_vScrollbar.inject_mouse_drag(ev.Position)) {
        force_redraw(this->name() + ": vertical scrollbar dragged");
        ev.Handled = true;
    }
}

void vscroll_widget::on_mouse_up(input::mouse::button_event const& ev)
{
    widget::on_mouse_up(ev);

    if (ev.Button == parent_form()->Controls->PrimaryMouseButton) {
        _vScrollbar.inject_mouse_up(ev.Position);
        force_redraw(this->name() + ": mouse up");
        ev.Handled = true;
    }
}

void vscroll_widget::on_mouse_wheel(input::mouse::wheel_event const& ev)
{
    widget::on_mouse_wheel(ev);

    if (!_vScrollbar.Visible) { return; }

    if (auto const* style {current_style<vscroll_widget::style>()}) {
        orientation  orien {};
        bool         invert {false};
        milliseconds delay {};

        if (ev.Scroll.Y != 0) {
            orien  = orientation::Vertical;
            invert = ev.Scroll.Y > 0;
            delay  = style->VScrollBar.Bar.Delay;
        }

        f32 const diff {get_scroll_content_height() / get_scroll_item_count() * (invert ? -5 : 5)};
        if (orien == orientation::Vertical) {
            _vScrollbar.set_target_value(_vScrollbar.target_value() + diff, delay);
        }
        ev.Handled = true;
    }
}

void vscroll_widget::on_update(milliseconds deltaTime)
{
    _vScrollbar.update(deltaTime);
}

void vscroll_widget::offset_content(rect_f& bounds, bool isHitTest) const
{
    widget::offset_content(bounds, isHitTest);

    if (!isHitTest) {
        if (auto const* style {current_style<vscroll_widget::style>()}) {
            if (_vScrollbar.Visible) {
                bounds.Size.Width -= style->VScrollBar.Bar.Size.calc(bounds.width());
            }
        }
    }
}

auto vscroll_widget::get_scrollbar_value() const -> f32
{
    return _vScrollbar.current_value();
}

void vscroll_widget::set_scrollbar_value(f32 value)
{
    _vScrollbar.set_target_value(value, milliseconds {0});
}

} // namespace ui
