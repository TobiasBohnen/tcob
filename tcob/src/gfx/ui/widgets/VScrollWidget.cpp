// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/ui/widgets/VScrollWidget.hpp"

#include "tcob/gfx/ui/Form.hpp"
#include "tcob/gfx/ui/WidgetPainter.hpp"

namespace tcob::gfx::ui {

void vscroll_widget::style::Transition(style& target, style const& left, style const& right, f64 step)
{
    widget_style::Transition(target, left, right, step);

    element::scrollbar::Transition(target.VScrollBar, left.VScrollBar, right.VScrollBar, step);
}

vscroll_widget::vscroll_widget(init const& wi)
    : widget {wi}
    , _vScrollbar {*this, orientation::Vertical}
{
}

void vscroll_widget::prepare_redraw()
{
    widget::prepare_redraw();

    _vScrollbar.Min = 0;
    _vScrollbar.Max = get_scroll_max_value();
}

auto vscroll_widget::get_scroll_max_value() const -> f32
{
    return std::max(0.0f, get_scroll_content_height() - content_bounds().height());
}

void vscroll_widget::on_styles_changed()
{
    widget::on_styles_changed();

    _vScrollbar.reset();
}

void vscroll_widget::paint_scrollbar(widget_painter& painter, rect_f& rect)
{
    auto const* style {dynamic_cast<vscroll_widget::style const*>(current_style())};

    _vScrollbar.Visible = get_scroll_content_height() - 1 > rect.height();
    auto const  thumbFlags {!_vScrollbar.is_mouse_over_thumb() ? widget_flags {}
                                : flags().Active               ? widget_flags {.Active = true}
                                                               : widget_flags {.Hover = true}};
    thumb_style thumbStyle;
    update_sub_style(thumbStyle, -2, style->VScrollBar.ThumbClass, thumbFlags);
    _vScrollbar.paint(painter, style->VScrollBar, thumbStyle.Thumb, rect);
}

void vscroll_widget::on_mouse_hover(input::mouse::motion_event const& ev)
{
    _vScrollbar.mouse_hover(ev.Position);
    if (_vScrollbar.is_mouse_over()) {
        force_redraw(this->name() + ": scrollbar mouse hover");
        ev.Handled = true;
    }
}

void vscroll_widget::on_mouse_down(input::mouse::button_event const& ev)
{
    if (ev.Button == parent_form()->Controls->PrimaryMouseButton) {
        if (_vScrollbar.is_mouse_over()) {
            _vScrollbar.mouse_down(ev.Position);
            force_redraw(this->name() + ": mouse down");
            ev.Handled = true;
        }
    }
}

void vscroll_widget::on_mouse_drag(input::mouse::motion_event const& ev)
{
    _vScrollbar.mouse_drag(ev.Position);
    if (_vScrollbar.is_dragging()) {
        force_redraw(this->name() + ": vertical scrollbar dragged");
        ev.Handled = true;
    }
}

void vscroll_widget::on_mouse_up(input::mouse::button_event const& ev)
{
    if (ev.Button == parent_form()->Controls->PrimaryMouseButton) {
        _vScrollbar.mouse_up(ev.Position);
        force_redraw(this->name() + ": mouse up");
        ev.Handled = true;
    }
}

void vscroll_widget::on_mouse_wheel(input::mouse::wheel_event const& ev)
{
    if (!_vScrollbar.Visible) { return; }
    if (ev.Scroll.Y == 0) { return; }

    bool const         invert {ev.Scroll.Y > 0};
    milliseconds const delay {dynamic_cast<vscroll_widget::style const*>(current_style())->VScrollBar.Bar.Delay};
    _vScrollbar.start_scroll(_vScrollbar.target_value() + (invert ? -get_scroll_distance() : get_scroll_distance()), delay);

    ev.Handled = true;
}

void vscroll_widget::on_update(milliseconds deltaTime)
{
    _vScrollbar.update(deltaTime);
}

void vscroll_widget::offset_content(rect_f& bounds, bool isHitTest) const
{
    widget::offset_content(bounds, isHitTest);

    // subtract scrollbar from content
    if (isHitTest) { return; }
    if (!_vScrollbar.Visible) { return; }

    bounds.Size.Width -= dynamic_cast<vscroll_widget::style const*>(current_style())->VScrollBar.Bar.Size.calc(bounds.width());
}

auto vscroll_widget::get_scrollbar_value() const -> f32
{
    return _vScrollbar.current_value();
}

void vscroll_widget::set_scrollbar_value(f32 value)
{
    _vScrollbar.start_scroll(value, milliseconds {0});
}

} // namespace ui
