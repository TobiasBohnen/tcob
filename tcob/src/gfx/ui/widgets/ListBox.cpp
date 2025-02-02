// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/ui/widgets/ListBox.hpp"

#include "tcob/core/StringUtils.hpp"
#include "tcob/gfx/ui/Form.hpp"
#include "tcob/gfx/ui/WidgetPainter.hpp"

namespace tcob::gfx::ui {

static constexpr isize INVALID {-1};

list_box::list_box(init const& wi)
    : vscroll_widget {wi}
    , SelectedItemIndex {{[this](isize val) -> isize { return std::clamp<isize>(val, INVALID, std::ssize(get_items()) - 1); }}}
    , HoveredItemIndex {{[this](isize val) -> isize { return std::clamp<isize>(val, INVALID, std::ssize(get_items()) - 1); }}}
{
    SelectedItemIndex.Changed.connect([this](auto const&) { force_redraw(this->name() + ": SelectedItem changed"); });
    SelectedItemIndex(INVALID);
    HoveredItemIndex.Changed.connect([this](auto const&) { force_redraw(this->name() + ": HoveredItem changed"); });
    HoveredItemIndex(INVALID);

    Filter.Changed.connect([this](auto const& val) {
        HoveredItemIndex  = INVALID;
        SelectedItemIndex = INVALID;
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
    list_item const litem {.Text = item, .UserData = {}};
    add_item(litem);
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
    SelectedItemIndex = INVALID;
    HoveredItemIndex  = INVALID;
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

    f32 value {0.0f};
    if (auto const* style {current_style<list_box::style>()}) {
        rect_f const listRect {content_bounds()};
        f32 const    itemHeight {style->ItemHeight.calc(listRect.height())};
        auto const   scrollMax {get_scroll_max_value()};

        for (i32 i {0}; i < SelectedItemIndex; ++i) {
            rect_f const itemRect {get_item_rect(i, itemHeight, listRect)};
            value += itemRect.height();
            if (value >= scrollMax) {
                value = scrollMax;
                break;
            }
        }
    }

    set_scrollbar_value(value);
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

void list_box::paint_content(widget_painter& painter, rect_f const& rect)
{
    if (auto const* style {current_style<list_box::style>()}) {
        _itemRectCache.clear();

        rect_f const listRect {rect};
        f32 const    itemHeight {style->ItemHeight.calc(listRect.height())};

        auto const paint_item {[&](isize i) {
            auto const&  itemStyle {get_item_style(i)};
            rect_f const itemRect {get_item_rect(i, itemHeight, listRect)};
            if (itemRect.bottom() > listRect.top() && itemRect.top() < listRect.bottom()) {
                painter.draw_item(itemStyle->Item, itemRect, get_items()[i].Text);
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
    HoveredItemIndex = INVALID;

    vscroll_widget::on_mouse_leave();
}

void list_box::on_mouse_hover(input::mouse::motion_event const& ev)
{
    HoveredItemIndex = INVALID;

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
        if (HoveredItemIndex == INVALID) { return; }
        if (SelectedItemIndex != HoveredItemIndex()) {
            SelectedItemIndex = HoveredItemIndex();
            ev.Handled        = true;
        } // else SelectedItemClicked event?
    }
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
    auto const* style {current_style<list_box::style>()};
    return index == SelectedItemIndex ? get_sub_style<item_style>(style->ItemClass, {.Active = true})
        : index == HoveredItemIndex   ? get_sub_style<item_style>(style->ItemClass, {.Hover = true})
                                      : get_sub_style<item_style>(style->ItemClass, {});
}

auto list_box::get_scroll_content_height() const -> f32
{
    if (_items.empty()) { return 0; }

    f32 retValue {0.0f};
    if (auto const* style {current_style<list_box::style>()}) {
        rect_f const listRect {content_bounds()};
        f32 const    itemHeight {style->ItemHeight.calc(listRect.height())};
        for (i32 i {0}; i < item_count(); ++i) {
            rect_f const itemRect {get_item_rect(i, itemHeight, listRect)};
            retValue += itemRect.height();
        }
    }

    return retValue;
}

auto list_box::get_scroll_item_count() const -> isize
{
    return item_count();
}

auto list_box::get_items() const -> std::vector<list_item> const&
{
    return Filter->empty() ? _items : _filteredItems;
}

auto list_box::attributes() const -> widget_attributes
{
    auto retValue {vscroll_widget::attributes()};
    if (SelectedItemIndex >= 0 && SelectedItemIndex < std::ssize(get_items())) {
        retValue["selected"] = selected_item().Text;
    }
    return retValue;
}

} // namespace ui
