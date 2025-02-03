// Copyright (c) 2025 Tobias Bohnen
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

// TODO: datasource
class TCOB_API drop_down_list : public widget {
public:
    class TCOB_API style : public widget_style {
    public:
        element::text Text;

        utf8_string NavArrowClass {"nav_arrows"};

        utf8_string ItemClass {"list_items"};
        length      ItemHeight {};
        isize       VisibleItemCount {5};

        element::scrollbar VScrollBar;
    };

    explicit drop_down_list(init const& wi);

    prop_val<isize> SelectedItemIndex;
    prop_val<isize> HoveredItemIndex;

    void prepare_redraw() override;

    void add_item(utf8_string const& item);
    void add_item(list_item const& item);
    void clear_items();

    auto select_item(utf8_string const& item) -> bool;

    auto get_item_at(isize index) const -> list_item const&;
    auto selected_item() const -> list_item const&;
    auto item_count() const -> isize;

protected:
    void on_styles_changed() override;

    void on_paint(widget_painter& painter) final;

    void on_mouse_leave() override;
    void on_mouse_hover(input::mouse::motion_event const& ev) override;
    void on_mouse_down(input::mouse::button_event const& ev) override;
    void on_mouse_drag(input::mouse::motion_event const& ev) override;
    void on_mouse_up(input::mouse::button_event const& ev) override;
    void on_mouse_wheel(input::mouse::wheel_event const& ev) override;

    void on_focus_lost() override;

    void on_update(milliseconds deltaTime) override;

    void offset_content(rect_f& bounds, bool isHitTest) const override;

    auto attributes() const -> widget_attributes override;

private:
    auto get_item_height() const -> f32;

    void set_extended(bool v);

    auto get_item_rect(isize index, f32 itemHeight, rect_f const& listRect) const -> rect_f;
    auto get_item_style(isize index) const -> item_style*;

    std::vector<list_item>            _items;
    std::unordered_map<isize, rect_f> _itemRectCache;

    bool _isExtended {false};
    bool _mouseOverBox {false};

    scrollbar _vScrollbar;
};
}
