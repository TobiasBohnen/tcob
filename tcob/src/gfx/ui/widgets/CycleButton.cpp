// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/ui/widgets/CycleButton.hpp"

#include <algorithm>
#include <iterator>
#include <tuple>
#include <vector>

#include "tcob/core/Common.hpp"
#include "tcob/core/Rect.hpp"
#include "tcob/core/input/Input.hpp"
#include "tcob/gfx/ui/Style.hpp"
#include "tcob/gfx/ui/StyleElements.hpp"
#include "tcob/gfx/ui/UI.hpp"
#include "tcob/gfx/ui/WidgetPainter.hpp"
#include "tcob/gfx/ui/component/Item.hpp"
#include "tcob/gfx/ui/widgets/Widget.hpp"

namespace tcob::ui {

void cycle_button::style::Transition(style& target, style const& from, style const& to, f64 step)
{
    widget_style::Transition(target, from, to, step);

    target.ItemHeight = length::Lerp(from.ItemHeight, to.ItemHeight, step);
    target.Bar.lerp(from.Bar, to.Bar, step);
    target.GapRatio = helper::lerp(from.GapRatio, to.GapRatio, step);
}

cycle_button::cycle_button(init const& wi)
    : widget {wi}
    , SelectedItemIndex {{[this](isize val) -> isize { return std::clamp<isize>(val, INVALID_INDEX, std::ssize(*Items) - 1); }}}
{
    _tween.Changed.connect([this] { queue_redraw(); });
    _tween.reset(1);

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

auto cycle_button::selected_item() const -> item const&
{
    return Items->at(SelectedItemIndex);
}

void cycle_button::on_draw(widget_painter& painter)
{
    rect_f const rect {draw_background(_style, painter)};

    scoped_scissor const guard {painter, this};

    if (SelectedItemIndex != INVALID_INDEX) {
        // item
        rect_f    itemRect {rect};
        f32 const itemHeight {_style.ItemHeight.calc(rect.height())};
        itemRect.Size.Height = itemHeight;

        item_style itemStyle {};
        prepare_sub_style(itemStyle, 0, _style.ItemClass, {});
        painter.draw_item(itemStyle.Item, itemRect, selected_item());

        // bar
        rect_f barRect {rect};
        barRect.Position.Y += itemHeight;
        barRect.Size.Height -= itemHeight;
        if (barRect.height() > 0) {
            isize const itemCount {std::ssize(*Items) * 2};

            f32 const unit {1.0f / itemCount};
            f32 const gapSize {unit * _style.GapRatio};
            f32 const barSize {(2 * unit) - gapSize};

            std::vector<f32>               stops(itemCount + 2);
            std::vector<bar_element::type> stopPattern(stops.size() - 1);
            stops[0]       = 0;
            stops[1]       = gapSize / 2;
            stopPattern[0] = bar_element::type::Empty;

            for (isize i {2}; i < std::ssize(stops); ++i) {
                if (i % 2 == 0) {
                    isize const idx {(i - 1) / 2};
                    stops[i] = stops[i - 1] + barSize;
                    if ((idx == SelectedItemIndex - 1 && _tween.current_value() < 0.5f)
                        || (idx == SelectedItemIndex && _tween.current_value() >= 0.5f)) {
                        stopPattern[i - 1] = bar_element::type::High;
                    } else {
                        stopPattern[i - 1] = bar_element::type::Low;
                    }
                } else {
                    stops[i]           = stops[i - 1] + gapSize;
                    stopPattern[i - 1] = bar_element::type::Empty;
                }
            }
            std::ignore = painter.draw_bar(
                _style.Bar,
                barRect,
                {.Orientation = orientation::Horizontal,
                 .Position    = bar_element::position::CenterOrMiddle,
                 .Stops       = stops,
                 .StopPattern = stopPattern});
        }
    }
}

void cycle_button::on_update(milliseconds deltaTime)
{
    _tween.update(deltaTime);
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
    SelectedItemIndex = (SelectedItemIndex + 1) % Items->size();
    if (SelectedItemIndex == 0) {
        _tween.reset(1);
    } else {
        _tween.reset(0);
        _tween.start(1, _style.Bar.Delay);
    }
}

}
