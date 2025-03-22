// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/ui/widgets/ListBox.hpp"

#include <algorithm>
#include <cctype>
#include <iterator>
#include <vector>

#include "tcob/core/Rect.hpp"
#include "tcob/core/StringUtils.hpp"
#include "tcob/core/input/Input.hpp"
#include "tcob/gfx/ui/Style.hpp"
#include "tcob/gfx/ui/UI.hpp"
#include "tcob/gfx/ui/WidgetPainter.hpp"
#include "tcob/gfx/ui/widgets/VScrollWidget.hpp"

namespace tcob::ui {

auto static case_insensitive_contains(string_view lhs, string_view rhs) -> bool
{
    return std::search( // NOLINT(modernize-use-ranges)
               lhs.begin(), lhs.end(),
               rhs.begin(), rhs.end(),
               [](char a, char b) { return std::toupper(a) == std::toupper(b); })
        != lhs.end();
}

void list_box::style::Transition(style& target, style const& left, style const& right, f64 step)
{
    vscroll_widget::style::Transition(target, left, right, step);

    target.ItemHeight = length::Lerp(left.ItemHeight, right.ItemHeight, step);
}

list_box::list_box(init const& wi)
    : vscroll_widget {wi}
    , SelectedItemIndex {{[this](isize val) -> isize { return std::clamp<isize>(val, INVALID_INDEX, std::ssize(get_items()) - 1); }}}
    , HoveredItemIndex {{[this](isize val) -> isize { return std::clamp<isize>(val, INVALID_INDEX, std::ssize(get_items()) - 1); }}}
{
    SelectedItemIndex.Changed.connect([this](auto const&) { request_redraw(this->name() + ": SelectedItem changed"); });
    SelectedItemIndex(INVALID_INDEX);
    HoveredItemIndex.Changed.connect([this](auto const&) { request_redraw(this->name() + ": HoveredItem changed"); });
    HoveredItemIndex(INVALID_INDEX);

    Filter.Changed.connect([this](auto const& val) {
        HoveredItemIndex  = INVALID_INDEX;
        SelectedItemIndex = INVALID_INDEX;
        set_scrollbar_value(0);

        _filteredItems.clear();
        if (!val.empty()) {
            _filteredItems.reserve(_items.size());
            for (i32 i {0}; i < std::ssize(_items); ++i) {
                if (case_insensitive_contains(_items[i].Text, val)) {
                    _filteredItems.push_back(_items[i]);
                }
            }
        }
        request_redraw(this->name() + ": Filter changed");
    });

    Class("list_box");
}

void list_box::add_item(utf8_string const& item)
{
    add_item({.Text = item, .Icon = {}, .UserData = {}});
}

void list_box::add_item(list_item const& item)
{
    _items.push_back(item);
    if (!Filter->empty() && case_insensitive_contains(item.Text, Filter())) {
        _filteredItems.push_back(item);
    }
    request_redraw(this->name() + ": item added");
}

void list_box::clear_items()
{
    _items.clear();
    _filteredItems.clear();
    clear_sub_styles();

    SelectedItemIndex = INVALID_INDEX;
    HoveredItemIndex  = INVALID_INDEX;
    request_redraw(this->name() + ": items cleared");
}

auto list_box::select_item(utf8_string const& item) -> bool
{
    auto const& items {get_items()};
    for (isize i {0}; i < std::ssize(items); ++i) {
        if (items[i].Text == item) {
            SelectedItemIndex = i;
            return true;
        }
    }

    return false;
}

void list_box::scroll_to_selected()
{
    if (SelectedItemIndex == INVALID_INDEX) { return; }

    _scrollToSelected = true;
}

auto list_box::get_item_at(isize index) const -> list_item const&
{
    return get_items().at(static_cast<usize>(index));
}

auto list_box::selected_item() const -> list_item const&
{
    return get_items().at(SelectedItemIndex());
}

auto list_box::item_count() const -> isize
{
    return std::ssize(get_items());
}

void list_box::prepare_redraw()
{
    apply_style(_style);
    vscroll_widget::prepare_redraw();
}

void list_box::on_draw(widget_painter& painter)
{
    rect_f rect {draw_background(_style, painter)};

    // scrollbar
    draw_scrollbar(painter, rect);

    // content
    scissor_guard const guard {painter, this};

    _itemRectCache.clear();

    rect_f const listRect {rect};
    f32 const    itemHeight {_style.ItemHeight.calc(listRect.height())};
    _visibleItems = static_cast<isize>(listRect.height() / itemHeight);
    auto const scrollOffset {scrollbar_offset()};

    auto const paintItem {[&](isize i) {
        rect_f itemRect {listRect};
        itemRect.Size.Height = itemHeight;
        itemRect.Position.Y  = listRect.top() + (itemRect.height() * i) - scrollOffset;

        if (itemRect.bottom() > listRect.top() && itemRect.top() < listRect.bottom()) {
            item_style itemStyle {};
            apply_sub_style(itemStyle, i, _style.ItemClass, {.Active = i == SelectedItemIndex, .Hover = i == HoveredItemIndex});
            painter.draw_item(itemStyle.Item, itemRect, get_items()[i]);
            _itemRectCache[i] = itemRect;
        } else {
            reset_sub_style(i, _style.ItemClass, {.Active = i == SelectedItemIndex, .Hover = i == HoveredItemIndex});
        }
    }};

    isize const size {item_count()};
    for (i32 i {0}; i < size; ++i) {
        if (i == HoveredItemIndex || i == SelectedItemIndex) { continue; }
        paintItem(i);
    }

    if (SelectedItemIndex != INVALID_INDEX) {
        paintItem(SelectedItemIndex);
    }
    if (HoveredItemIndex != INVALID_INDEX && SelectedItemIndex != HoveredItemIndex) {
        paintItem(HoveredItemIndex);
    }
}

void list_box::on_update(milliseconds deltaTime)
{
    vscroll_widget::on_update(deltaTime);

    // scroll to selected
    if (SelectedItemIndex != INVALID_INDEX && _scrollToSelected && !_itemRectCache.empty()) { // delay scroll to selected after first paint
        f32 const itemHeight {_style.ItemHeight.calc(content_bounds().height())};
        set_scrollbar_value(std::min(itemHeight * SelectedItemIndex, get_scroll_max()));
        _scrollToSelected = false;
    }
}

void list_box::on_key_down(input::keyboard::event const& ev)
{
    using namespace tcob::enum_ops;

    // select based on first char
    auto const  kc {static_cast<char>(ev.KeyCode)};
    auto const& items {get_items()};
    if (Filter->empty() && !items.empty() && SelectedItemIndex >= 0 && kc >= 'a' && kc <= 'z') { // TODO: make optional
        bool const reverse {(ev.KeyMods & input::key_mod::LeftShift) == input::key_mod::LeftShift};

        isize idx {SelectedItemIndex()};
        do {
            if (reverse) {
                idx--;
                if (idx < 0) { idx = std::ssize(items) - 1; }
            } else {
                idx++;
                if (idx >= std::ssize(items)) { idx = 0; }
            }
            if (!items[idx].Text.empty() && std::tolower(items[idx].Text[0]) == kc) {
                SelectedItemIndex = idx;
                scroll_to_selected();
                ev.Handled = true;
                break;
            }
        } while (idx != SelectedItemIndex);
    }
}

void list_box::on_mouse_leave()
{
    HoveredItemIndex = INVALID_INDEX;

    vscroll_widget::on_mouse_leave();
}

void list_box::on_mouse_hover(input::mouse::motion_event const& ev)
{
    vscroll_widget::on_mouse_hover(ev);
    if (ev.Handled) { return; }

    auto const mp {global_to_parent(*this, ev.Position)};
    for (auto const& kvp : _itemRectCache) {
        if (!kvp.second.contains(mp)) { continue; }
        HoveredItemIndex = kvp.first;
        ev.Handled       = true;
        return;
    }

    HoveredItemIndex = INVALID_INDEX;
}

void list_box::on_mouse_down(input::mouse::button_event const& ev)
{
    vscroll_widget::on_mouse_down(ev);

    if (ev.Button == controls().PrimaryMouseButton) {
        if (HoveredItemIndex == INVALID_INDEX) { return; }
        if (SelectedItemIndex != HoveredItemIndex()) {
            SelectedItemIndex = HoveredItemIndex();
            ev.Handled        = true;
        }
    }
}

void list_box::on_mouse_wheel(input::mouse::wheel_event const& ev)
{
    HoveredItemIndex = INVALID_INDEX;

    vscroll_widget::on_mouse_wheel(ev);
}

auto list_box::get_items() const -> std::vector<list_item> const&
{
    return Filter->empty() ? _items : _filteredItems;
}

auto list_box::attributes() const -> widget_attributes
{
    auto retValue {vscroll_widget::attributes()};

    auto const& items {get_items()};
    auto const  size {std::ssize(items)};

    retValue["selected_index"] = SelectedItemIndex();
    if (SelectedItemIndex >= 0 && SelectedItemIndex < size) {
        retValue["selected"] = items.at(SelectedItemIndex).Text;
    }
    retValue["hover_index"] = HoveredItemIndex();
    if (HoveredItemIndex >= 0 && HoveredItemIndex < size) {
        retValue["hover"] = items.at(HoveredItemIndex()).Text;
    }

    return retValue;
}

auto list_box::get_scroll_content_height() const -> f32
{
    if (_items.empty()) { return 0; }

    f32       retValue {0.0f};
    f32 const itemHeight {_style.ItemHeight.calc(content_bounds().height())};
    for (i32 i {0}; i < item_count(); ++i) { retValue += itemHeight; }

    return retValue;
}

auto list_box::get_scroll_distance() const -> f32
{
    return _style.ItemHeight.calc(content_bounds().height()) * _visibleItems / get_scroll_max();
}

} // namespace ui
