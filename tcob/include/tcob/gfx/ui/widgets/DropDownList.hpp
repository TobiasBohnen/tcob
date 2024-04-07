// Copyright (c) 2023 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include "tcob/tcob_config.hpp"

#include "tcob/gfx/ui/Scrollbar.hpp"
#include "tcob/gfx/ui/UI.hpp"
#include "tcob/gfx/ui/widgets/Widget.hpp"

namespace tcob::gfx::ui {
////////////////////////////////////////////////////////////

class TCOB_API drop_down_list : public widget {
    friend class scrollbar<drop_down_list>;

public:
    class TCOB_API style : public background_style {
    public:
        element::text Text;

        length             ItemHeight {};
        utf8_string        ItemClass {"list_items"};
        element::scrollbar VScrollBar;
    };

    explicit drop_down_list(init const& wi);

    prop_val<isize> SelectedItemIndex;
    prop_val<isize> HoveredItemIndex;
    prop<isize>     VisibleItems;

    void add_item(utf8_string const& item);
    void clear_items();

    auto select_item(utf8_string const& item) -> bool;

    auto get_item_at(isize index) const -> utf8_string const&;
    auto get_selected_item() const -> utf8_string const&;
    auto get_item_count() const -> isize;

protected:
    void on_styles_changed() override;

    void on_paint(widget_painter& painter) final;

    void on_mouse_leave() override;
    void on_mouse_hover(input::mouse::motion_event& ev) override;
    void on_mouse_down(input::mouse::button_event& ev) override;
    void on_mouse_drag(input::mouse::motion_event& ev) override;
    void on_mouse_up(input::mouse::button_event& ev) override;
    void on_mouse_wheel(input::mouse::wheel_event& ev) override;
    void on_double_click() override;

    void on_focus_lost() override;

    void on_update(milliseconds deltaTime) override;

    void offset_content(rect_f& bounds, bool isHitTest) const override;

    auto get_properties() const -> widget_attributes override;

private:
    auto get_item_height() const -> f32;

    auto requires_scroll(orientation orien, rect_f const& rect) const -> bool;
    auto get_scroll_min_value(orientation orien) const -> f32;
    auto get_scroll_max_value(orientation orien) const -> f32;
    auto get_scroll_style(orientation orien) const -> element::scrollbar*;

    void set_extended(bool v);

    void paint_item(widget_painter& painter, rect_f& listRect, f32 itemHeight, isize i);

    auto get_item_rect(isize index, f32 itemHeight, rect_f const& listRect) const -> rect_f;
    auto get_item_style(isize index) const -> std::shared_ptr<item_style>;

    std::vector<utf8_string> _items;
    bool                     _isExtended {false};
    bool                     _mouseOverBox {false};

    scrollbar<drop_down_list> _vScrollbar;
};
}
