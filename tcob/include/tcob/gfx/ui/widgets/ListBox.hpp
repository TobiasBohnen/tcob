// Copyright (c) 2023 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include "tcob/tcob_config.hpp"

#include "tcob/gfx/ui/UI.hpp"
#include "tcob/gfx/ui/widgets/VScrollWidget.hpp"

namespace tcob::gfx::ui {
////////////////////////////////////////////////////////////

class TCOB_API list_box : public vscroll_widget {
public:
    class TCOB_API style : public vscroll_widget::style {
    public:
        length      ItemHeight {};
        utf8_string ItemClass {"list_items"};
    };

    explicit list_box(init const& wi);

    prop_val<isize> SelectedItemIndex;
    prop_val<isize> HoveredItemIndex;

    void add_item(utf8_string const& item);
    void clear_items();

    auto select_item(utf8_string const& item) -> bool;
    void scroll_to_selected();

    auto get_item_at(isize index) const -> utf8_string const&;
    auto get_selected_item() const -> utf8_string const&;
    auto get_item_count() const -> isize;

protected:
    void paint_content(widget_painter& painter, rect_f const& rect) override;

    void on_key_down(input::keyboard::event& ev) override;

    void on_mouse_leave() override;
    void on_mouse_hover(input::mouse::motion_event& ev) override;
    void on_mouse_down(input::mouse::button_event& ev) override;

    auto get_properties() const -> widget_attributes override;

    auto get_list_height() const -> f32 override;
    auto get_list_item_count() const -> isize override;

private:
    void paint_item(widget_painter& painter, rect_f& listRect, f32 itemHeight, isize i);

    auto get_item_rect(isize index, f32 itemHeight, rect_f const& rect) const -> rect_f;
    auto get_item_style(isize index) const -> std::shared_ptr<item_style>;

    std::vector<utf8_string> _items;
};
}
