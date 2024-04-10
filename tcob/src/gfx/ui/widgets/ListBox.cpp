// Copyright (c) 2023 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/ui/widgets/ListBox.hpp"

#include "tcob/gfx/ui/Form.hpp"
#include "tcob/gfx/ui/WidgetPainter.hpp"

namespace tcob::gfx::ui {

list_box::list_box(init const& wi)
    : vscroll_widget {wi}
    , SelectedItemIndex {{[&](isize val) -> isize { return std::clamp<isize>(val, -1, std::ssize(_items) - 1); }}}
    , HoveredItemIndex {{[&](isize val) -> isize { return std::clamp<isize>(val, -1, std::ssize(_items) - 1); }}}
{
    SelectedItemIndex.Changed.connect([&](auto const&) { force_redraw(get_name() + ": SelectedItem changed"); });
    SelectedItemIndex(-1);
    HoveredItemIndex.Changed.connect([&](auto const&) { force_redraw(get_name() + ": HoveredItem changed"); });
    HoveredItemIndex(-1);

    Class("list_box");
}

void list_box::add_item(utf8_string const& item)
{
    _items.push_back(item);
    force_redraw(get_name() + ": item added");
}

void list_box::clear_items()
{
    _items.clear();
    force_redraw(get_name() + ": items cleared");
}

auto list_box::select_item(utf8_string const& item) -> bool
{
    for (isize i {0}; i < std::ssize(_items); ++i) {
        if (_items[i] == item) {
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
    if (auto const* style {get_style<list_box::style>()}) {
        rect_f const listRect {get_content_bounds()};
        f32 const    itemHeight {style->ItemHeight.calc(listRect.Height)};
        auto const   scrollMax {get_scroll_max_value(orientation::Vertical)};

        for (i32 i {0}; i < SelectedItemIndex; ++i) {
            rect_f const itemRect {get_item_rect(i, itemHeight, listRect)};
            value += itemRect.Height;
            if (value >= scrollMax) {
                value = scrollMax;
                break;
            }
        }
    }

    set_scrollbar_value(value);
}

auto list_box::get_item_at(isize index) const -> utf8_string const&
{
    return _items.at(static_cast<usize>(index));
}

auto list_box::get_selected_item() const -> utf8_string const&
{
    return _items.at(SelectedItemIndex);
}

auto list_box::get_item_count() const -> isize
{
    return std::ssize(_items);
}

void list_box::paint_item(widget_painter& painter, rect_f& listRect, f32 itemHeight, isize i)
{
    auto const&  itemStyle {get_item_style(i)};
    rect_f const itemRect {get_item_rect(i, itemHeight, listRect)};
    if (itemRect.bottom() > 0 && itemRect.top() < listRect.bottom()) {
        painter.draw_item(itemStyle->Item, itemRect, _items[i]);
    }
}

void list_box::paint_content(widget_painter& painter, rect_f const& rect)
{
    if (auto const* style {get_style<list_box::style>()}) {
        rect_f listRect {rect};

        f32 const itemHeight {style->ItemHeight.calc(listRect.Height)};

        for (i32 i {0}; i < std::ssize(_items); ++i) {
            if (i == HoveredItemIndex || i == SelectedItemIndex) { continue; }

            paint_item(painter, listRect, itemHeight, i);
        }

        if (SelectedItemIndex >= 0) {
            paint_item(painter, listRect, itemHeight, SelectedItemIndex);
        }
        if (HoveredItemIndex >= 0 && SelectedItemIndex != HoveredItemIndex) {
            paint_item(painter, listRect, itemHeight, HoveredItemIndex);
        }
    }
}

void list_box::on_key_down(input::keyboard::event& ev)
{
    using namespace tcob::enum_ops;

    auto const kc {static_cast<char>(ev.KeyCode)};
    if (!_items.empty() && SelectedItemIndex >= 0 && kc >= 'a' && kc <= 'z') { // TODO: make optional
        bool const reverse {(ev.KeyMods & input::key_mod::LeftShift) == input::key_mod::LeftShift};

        isize idx {SelectedItemIndex};
        do {
            if (reverse) {
                idx--;
                if (idx < 0) { idx = std::ssize(_items) - 1; }
            } else {
                idx++;
                if (idx >= std::ssize(_items)) { idx = 0; }
            }
            if (!_items[idx].empty() && std::tolower(_items[idx][0]) == kc) {
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
    vscroll_widget::on_mouse_leave();

    HoveredItemIndex = -1;
}

void list_box::on_mouse_hover(input::mouse::motion_event& ev)
{
    HoveredItemIndex = -1;

    vscroll_widget::on_mouse_hover(ev);

    if (auto const* style {get_style<list_box::style>()}) {
        rect_f const listRect {get_global_content_bounds()};
        if (listRect.contains(ev.Position)) {
            // over list
            f32 const itemHeight {style->ItemHeight.calc(listRect.Height)};
            for (i32 i {0}; i < std::ssize(_items); ++i) {
                rect_f const itemRect {get_item_rect(i, itemHeight, listRect)};
                if (itemRect.contains(ev.Position)) {
                    HoveredItemIndex = i;
                    ev.Handled       = true;
                    return;
                }
            }
        }
    }
}

void list_box::on_mouse_down(input::mouse::button_event& ev)
{
    vscroll_widget::on_mouse_down(ev);

    if (ev.Button == get_form()->Controls->PrimaryMouseButton) {
        if (HoveredItemIndex != -1) {
            if (SelectedItemIndex != HoveredItemIndex()) {
                SelectedItemIndex = HoveredItemIndex();
                ev.Handled        = true;
            } // else SelectedItemClicked event?
        }
    }
}

auto list_box::get_item_rect(isize index, f32 itemHeight, rect_f const& rect) const -> rect_f
{
    rect_f retValue {rect};
    retValue.Height = itemHeight;
    retValue.Y      = rect.Y + (retValue.Height * index) - get_scrollbar_value();
    return retValue;
}

auto list_box::get_item_style(isize index) const -> item_style*
{
    auto const* style {get_style<list_box::style>()};
    return index == SelectedItemIndex ? get_sub_style<item_style>(style->ItemClass, {.Active = true})
        : index == HoveredItemIndex   ? get_sub_style<item_style>(style->ItemClass, {.Hover = true})
                                      : get_sub_style<item_style>(style->ItemClass, {});
}

auto list_box::get_list_height() const -> f32
{
    f32 retValue {0.0f};

    if (auto const* style {get_style<list_box::style>()}) {
        rect_f const listRect {get_content_bounds()};
        f32 const    itemHeight {style->ItemHeight.calc(listRect.Height)};
        for (i32 i {0}; i < std::ssize(_items); ++i) {
            rect_f const itemRect {get_item_rect(i, itemHeight, listRect)};
            retValue += itemRect.Height;
        }
    }

    return retValue;
}

auto list_box::get_list_item_count() const -> isize
{
    return get_item_count();
}

auto list_box::get_attributes() const -> widget_attributes
{
    auto retValue {vscroll_widget::get_attributes()};
    if (SelectedItemIndex >= 0 && SelectedItemIndex < std::ssize(_items)) {
        retValue["selected"] = get_selected_item();
    }
    return retValue;
}

} // namespace ui
