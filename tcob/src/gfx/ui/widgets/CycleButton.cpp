// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/ui/widgets/CycleButton.hpp"

#include "tcob/gfx/ui/WidgetPainter.hpp"

namespace tcob::gfx::ui {

void cycle_button::style::Transition(style& target, style const& left, style const& right, f64 step)
{
    widget_style::Transition(target, left, right, step);

    element::text::Transition(target.Text, left.Text, right.Text, step);
}

cycle_button::cycle_button(init const& wi)
    : widget {wi}
    , SelectedItemIndex {{[this](isize val) -> isize { return std::clamp<isize>(val, INVALID_INDEX, std::ssize(_items) - 1); }}}
{
    SelectedItemIndex.Changed.connect([this](auto const&) { force_redraw(this->name() + ": SelectedItem changed"); });
    SelectedItemIndex(INVALID_INDEX);

    Class("cycle_button");
}

void cycle_button::add_item(utf8_string const& item)
{
    _items.push_back(item);
    force_redraw(this->name() + ": item added");
}

void cycle_button::clear_items()
{
    _items.clear();
    force_redraw(this->name() + ": items cleared");
}

auto cycle_button::select_item(utf8_string const& item) -> bool
{
    for (isize i {0}; i < std::ssize(_items); ++i) {
        if (_items[i] == item) {
            SelectedItemIndex = i;
            return true;
        }
    }

    return false;
}

auto cycle_button::get_item_at(isize index) const -> utf8_string const&
{
    return _items.at(static_cast<usize>(index));
}

auto cycle_button::selected_item() const -> utf8_string const&
{
    return _items.at(SelectedItemIndex());
}

auto cycle_button::item_count() const -> isize
{
    return std::ssize(_items);
}

void cycle_button::on_paint(widget_painter& painter)
{
    update_style(_style);

    rect_f rect {Bounds()};

    // background
    painter.draw_background_and_border(_style, rect, false);

    scissor_guard const guard {painter, this};

    // text
    if (_style.Text.Font && SelectedItemIndex >= 0) {
        painter.draw_text(_style.Text, rect, selected_item());
    }
}

void cycle_button::on_update(milliseconds /*deltaTime*/)
{
}

void cycle_button::on_click()
{
    SelectedItemIndex = (SelectedItemIndex() + 1) % item_count();
}

auto cycle_button::attributes() const -> widget_attributes
{
    auto retValue {widget::attributes()};

    retValue["selected_index"] = SelectedItemIndex();
    if (SelectedItemIndex >= 0 && SelectedItemIndex < std::ssize(_items)) {
        retValue["selected"] = selected_item();
    }

    return retValue;
}

} // namespace ui
