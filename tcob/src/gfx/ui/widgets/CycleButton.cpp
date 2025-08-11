// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/ui/widgets/CycleButton.hpp"

#include <algorithm>
#include <iterator>

#include "tcob/core/Rect.hpp"
#include "tcob/core/input/Input.hpp"
#include "tcob/gfx/ui/Style.hpp"
#include "tcob/gfx/ui/UI.hpp"
#include "tcob/gfx/ui/WidgetPainter.hpp"
#include "tcob/gfx/ui/component/Item.hpp"
#include "tcob/gfx/ui/widgets/Widget.hpp"

namespace tcob::ui {

void cycle_button::style::Transition(style& target, style const& left, style const& right, f64 step)
{
    widget_style::Transition(target, left, right, step);
}

cycle_button::cycle_button(init const& wi)
    : widget {wi}
    , SelectedItemIndex {{[this](isize val) -> isize { return std::clamp<isize>(val, INVALID_INDEX, std::ssize(*Items) - 1); }}}
{
    SelectedItemIndex.Changed.connect([this](auto const&) { queue_redraw(); });
    SelectedItemIndex(INVALID_INDEX);

    Items.Changed.connect([this](auto const& val) {
        if (std::ssize(val) <= SelectedItemIndex && SelectedItemIndex != INVALID_INDEX) {
            clear_sub_styles();
            SelectedItemIndex = INVALID_INDEX;
        }

        queue_redraw();
    });

    Class("cycle_button");
}

auto cycle_button::select_item(utf8_string const& item) -> bool
{
    for (isize i {0}; i < std::ssize(*Items); ++i) {
        if (Items[i].Text == item) {
            SelectedItemIndex = i;
            return true;
        }
    }

    return false;
}

auto cycle_button::get_item_at(isize index) const -> item const&
{
    return Items->at(static_cast<usize>(index));
}

auto cycle_button::selected_item() const -> item const&
{
    return Items->at(SelectedItemIndex);
}

auto cycle_button::item_count() const -> isize
{
    return std::ssize(*Items);
}

void cycle_button::on_draw(widget_painter& painter)
{
    rect_f rect {draw_background(_style, painter)};

    scissor_guard const guard {painter, this};

    // text //TODO: hover?
    if (SelectedItemIndex >= 0) {
        item_style itemStyle {};
        prepare_sub_style(itemStyle, 0, _style.ItemClass, {});
        painter.draw_item(itemStyle.Item, rect, selected_item());
    }
}

void cycle_button::on_update(milliseconds /*deltaTime*/)
{
}

void cycle_button::on_mouse_wheel(input::mouse::wheel_event const& /* ev */)
{
    select_next();
}

void cycle_button::on_click()
{
    select_next();
}

auto cycle_button::attributes() const -> widget_attributes
{
    auto retValue {widget::attributes()};

    retValue["selected_index"] = *SelectedItemIndex;
    if (SelectedItemIndex >= 0) {
        retValue["selected"] = selected_item().Text;
    }

    return retValue;
}

void cycle_button::select_next()
{
    if (Items->empty()) { return; }
    SelectedItemIndex = (SelectedItemIndex + 1) % item_count();
}

} // namespace ui
