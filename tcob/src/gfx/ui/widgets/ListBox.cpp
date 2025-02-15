// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/ui/widgets/ListBox.hpp"

#include "tcob/core/StringUtils.hpp"
#include "tcob/gfx/ui/Form.hpp"
#include "tcob/gfx/ui/WidgetPainter.hpp"

namespace tcob::gfx::ui {

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
    SelectedItemIndex.Changed.connect([this](auto const&) { force_redraw(this->name() + ": SelectedItem changed"); });
    SelectedItemIndex(INVALID_INDEX);
    HoveredItemIndex.Changed.connect([this](auto const&) { force_redraw(this->name() + ": HoveredItem changed"); });
    HoveredItemIndex(INVALID_INDEX);

    Filter.Changed.connect([this](auto const& val) {
        HoveredItemIndex  = INVALID_INDEX;
        SelectedItemIndex = INVALID_INDEX;
        set_scrollbar_value(0);

        _filteredItems.clear();
        if (!val.empty()) {
            _filteredItems.reserve(_items.size());
            for (i32 i {0}; i < std::ssize(_items); ++i) {
                if (helper::case_insensitive_contains(_items[i].Text, Filter())) {
                    _filteredItems.push_back(_items[i]);
                }
            }
        }
        force_redraw(this->name() + ": Filter changed");
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
    if (!Filter->empty() && helper::case_insensitive_contains(item.Text, Filter())) {
        _filteredItems.push_back(item);
    }
    force_redraw(this->name() + ": item added");
}

void list_box::clear_items()
{
    _items.clear();
    _filteredItems.clear();
    SelectedItemIndex = INVALID_INDEX;
    HoveredItemIndex  = INVALID_INDEX;
    force_redraw(this->name() + ": items cleared");
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
    if (SelectedItemIndex < 1) { return; }

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

void list_box::on_paint(widget_painter& painter)
{
    update_style(_style);

    rect_f rect {Bounds()};

    // background
    painter.draw_background_and_border(_style, rect, false);

    // scrollbar
    paint_scrollbar(painter, rect);

    // content
    scissor_guard const guard {painter, this};

    _itemRectCache.clear();

    rect_f const listRect {rect};
    f32 const    itemHeight {_style.ItemHeight.calc(listRect.height())};

    auto const paint_item {[&](isize i) {
        auto const&  itemStyle {get_item_style(i)};
        rect_f const itemRect {get_item_rect(i, itemHeight, listRect)};
        if (itemRect.bottom() > listRect.top() && itemRect.top() < listRect.bottom()) {
            painter.draw_item(itemStyle->Item, itemRect, get_items()[i]);
            _itemRectCache[i] = itemRect;
        }
    }};

    for (i32 i {0}; i < item_count(); ++i) {
        if (i == HoveredItemIndex || i == SelectedItemIndex) { continue; }
        paint_item(i);
    }

    if (SelectedItemIndex >= 0) {
        paint_item(SelectedItemIndex());
    }
    if (HoveredItemIndex >= 0 && SelectedItemIndex != HoveredItemIndex) {
        paint_item(HoveredItemIndex());
    }
}

void list_box::on_update(milliseconds deltaTime)
{
    vscroll_widget::on_update(deltaTime);

    if (_scrollToSelected && !_itemRectCache.empty()) { // delay scroll to selected after first paint
        rect_f const listRect {content_bounds()};
        f32 const    itemHeight {_style.ItemHeight.calc(listRect.height())};
        set_scrollbar_value(std::min(itemHeight * SelectedItemIndex, get_scroll_max_value()));
        _scrollToSelected = false;
    }
}

void list_box::on_key_down(input::keyboard::event const& ev)
{
    using namespace tcob::enum_ops;

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
    HoveredItemIndex = INVALID_INDEX;

    vscroll_widget::on_mouse_hover(ev);

    auto const mp {global_to_local(ev.Position)};

    for (auto const& kvp : _itemRectCache) {
        if (!kvp.second.contains(mp)) { continue; }
        HoveredItemIndex = kvp.first;
        ev.Handled       = true;
        return;
    }
}

void list_box::on_mouse_down(input::mouse::button_event const& ev)
{
    vscroll_widget::on_mouse_down(ev);

    if (ev.Button == parent_form()->Controls->PrimaryMouseButton) {
        if (HoveredItemIndex == INVALID_INDEX) { return; }
        if (SelectedItemIndex != HoveredItemIndex()) {
            SelectedItemIndex = HoveredItemIndex();
            ev.Handled        = true;
        } // else SelectedItemClicked event?
    }
}

void list_box::on_mouse_wheel(input::mouse::wheel_event const& ev)
{
    HoveredItemIndex = INVALID_INDEX;

    vscroll_widget::on_mouse_wheel(ev);
}

auto list_box::get_item_rect(isize index, f32 itemHeight, rect_f const& rect) const -> rect_f
{
    rect_f retValue {rect};
    retValue.Size.Height = itemHeight;
    retValue.Position.Y  = rect.top() + (retValue.height() * index) - get_scrollbar_value();
    return retValue;
}

auto list_box::get_item_style(isize index) const -> item_style*
{
    return index == SelectedItemIndex ? get_sub_style<item_style>(_style.ItemClass, {.Active = true})
        : index == HoveredItemIndex   ? get_sub_style<item_style>(_style.ItemClass, {.Hover = true})
                                      : get_sub_style<item_style>(_style.ItemClass, {});
}

auto list_box::get_scroll_content_height() const -> f32
{
    if (_items.empty()) { return 0; }

    f32          retValue {0.0f};
    rect_f const listRect {content_bounds()};
    f32 const    itemHeight {_style.ItemHeight.calc(listRect.height())};
    for (i32 i {0}; i < item_count(); ++i) {
        rect_f const itemRect {get_item_rect(i, itemHeight, listRect)};
        retValue += itemRect.height();
    }

    return retValue;
}

auto list_box::get_items() const -> std::vector<list_item> const&
{
    return Filter->empty() ? _items : _filteredItems;
}

auto list_box::attributes() const -> widget_attributes
{
    auto        retValue {vscroll_widget::attributes()};
    auto const& items {get_items()};
    auto const  size {std::ssize(items)};

    if (SelectedItemIndex >= 0 && SelectedItemIndex < size) {
        retValue["selected"] = items.at(SelectedItemIndex).Text;
    }
    if (HoveredItemIndex >= 0 && HoveredItemIndex < size) {
        retValue["hover"] = items.at(HoveredItemIndex()).Text;
    }
    return retValue;
}

auto list_box::get_style(bool update) -> vscroll_widget::style*
{
    if (update) { update_style(_style); }
    return &_style;
}

} // namespace ui
