// Copyright (c) 2023 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/ui/widgets/CycleButton.hpp"

#include "tcob/gfx/ui/WidgetPainter.hpp"

namespace tcob::gfx::ui {

cycle_button::cycle_button(init const& wi)
    : widget {wi}
    , SelectedItemIndex {{[&](i32 val) -> i32 { return std::clamp(val, -1, static_cast<i32>(_items.size()) - 1); }}}
{
    SelectedItemIndex.Changed.connect([&](auto const&) { force_redraw(get_name() + ": SelectedItem changed"); });
    SelectedItemIndex(-1);

    Class("cycle_button");
}

void cycle_button::add_item(utf8_string const& item)
{
    _items.push_back(item);
    force_redraw(get_name() + ": item added");
}

void cycle_button::clear_items()
{
    _items.clear();
    force_redraw(get_name() + ": items cleared");
}

auto cycle_button::get_item_at(i32 index) const -> utf8_string const&
{
    return _items.at(static_cast<usize>(index));
}

auto cycle_button::get_selected_item() const -> utf8_string const&
{
    return _items.at(SelectedItemIndex);
}

auto cycle_button::get_item_count() const -> isize
{
    return std::ssize(_items);
}

void cycle_button::on_paint(widget_painter& painter)
{
    if (auto const style {get_style<cycle_button::style>()}) {
        rect_f rect {Bounds()};

        // background
        painter.draw_background_and_border(*style, rect, false);

        scissor_guard const guard {painter, this};

        // text
        if (style->Text.Font && SelectedItemIndex >= 0) {
            painter.draw_text(style->Text, rect, get_selected_item());
        }
    }
}

void cycle_button::on_mouse_down(input::mouse::button_event& ev)
{
    if (SelectedItemIndex == get_item_count() - 1) {
        SelectedItemIndex = 0;
    } else {
        SelectedItemIndex = SelectedItemIndex + 1;
    }

    ev.Handled = true;
}

void cycle_button::on_update(milliseconds /*deltaTime*/)
{
}

auto cycle_button::get_properties() const -> widget_attributes
{
    auto retValue {widget::get_properties()};
    if (SelectedItemIndex >= 0 && SelectedItemIndex < std::ssize(_items)) {
        retValue["selected"] = get_selected_item();
    }
    return retValue;
}

} // namespace ui
