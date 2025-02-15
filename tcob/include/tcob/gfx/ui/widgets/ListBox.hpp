// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include "tcob/tcob_config.hpp"

#include "tcob/gfx/ui/UI.hpp"
#include "tcob/gfx/ui/widgets/VScrollWidget.hpp"

namespace tcob::gfx::ui {
////////////////////////////////////////////////////////////

// TODO: datasource

class TCOB_API list_box : public vscroll_widget {
public:
    class TCOB_API style : public vscroll_widget::style {
    public:
        utf8_string ItemClass {"list_items"};
        length      ItemHeight {};

        void static Transition(style& target, style const& left, style const& right, f64 step);
    };

    explicit list_box(init const& wi);

    prop_val<isize>   SelectedItemIndex;
    prop_val<isize>   HoveredItemIndex;
    prop<utf8_string> Filter;

    void add_item(utf8_string const& item);
    void add_item(list_item const& item);
    void clear_items();

    auto select_item(utf8_string const& item) -> bool;
    void scroll_to_selected();

    auto get_item_at(isize index) const -> list_item const&;
    auto selected_item() const -> list_item const&;
    auto item_count() const -> isize;

protected:
    void on_paint(widget_painter& painter) override;

    void on_update(milliseconds deltaTime) override;

    void on_key_down(input::keyboard::event const& ev) override;

    void on_mouse_leave() override;
    void on_mouse_hover(input::mouse::motion_event const& ev) override;
    void on_mouse_down(input::mouse::button_event const& ev) override;
    void on_mouse_wheel(input::mouse::wheel_event const& ev) override;

    auto attributes() const -> widget_attributes override;

    auto get_style(bool update) -> vscroll_widget::style* override;
    auto get_scroll_content_height() const -> f32 override;

private:
    auto get_items() const -> std::vector<list_item> const&;

    auto get_item_rect(isize index, f32 itemHeight, rect_f const& rect) const -> rect_f;
    auto get_item_style(isize index) const -> item_style*;

    std::vector<list_item>            _items;
    std::vector<list_item>            _filteredItems;
    std::unordered_map<isize, rect_f> _itemRectCache;
    bool                              _scrollToSelected {false};

    list_box::style _style;
};
}
