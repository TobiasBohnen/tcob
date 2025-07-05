// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/ui/widgets/DraggableWidget.hpp"

#include <optional>

#include "tcob/core/Point.hpp"
#include "tcob/core/ServiceLocator.hpp"
#include "tcob/core/input/Input.hpp"
#include "tcob/gfx/ui/Form.hpp"
#include "tcob/gfx/ui/UI.hpp"

namespace tcob::ui {

void draggable_widget::style::Transition(style& target, style const& left, style const& right, f64 step)
{
    widget_style::Transition(target, left, right, step);

    target.DragAlpha = static_cast<f32>(left.DragAlpha + ((right.DragAlpha - left.DragAlpha) * step));
}

draggable_widget::draggable_widget(init const& wi)
    : widget {wi}
{
}

void draggable_widget::on_mouse_drag(input::mouse::motion_event const& ev)
{
    if (Draggable) {
        _isDragging = true;
        queue_redraw(this->name() + ": dragging");
        ev.Handled = true;
    }
}

void draggable_widget::on_mouse_button_up(input::mouse::button_event const& ev)
{
    if (_isDragging) {
        Dropped({.Sender = this, .Target = form().find_widget_at(ev.Position).get(), .Position = ev.Position});
        queue_redraw(this->name() + ": dropped");
        ev.Handled = true;
    }

    _dragOffset = point_f::Zero;
    _isDragging = false;
}

void draggable_widget::on_mouse_button_down(input::mouse::button_event const& ev)
{
    _isDragging = false;

    if (Draggable && ev.Button == controls().PrimaryMouseButton) {
        _dragOffset = global_to_parent(*this, ev.Position) - drag_origin();
        _isDragging = true;
        ev.Handled  = true;
    }
}

auto draggable_widget::drag_offset() const -> std::optional<point_f>
{
    return _isDragging ? std::optional {global_to_parent(*this, locate_service<input::system>().mouse().get_position()) - _dragOffset}
                       : std::nullopt;
}

} // namespace ui
