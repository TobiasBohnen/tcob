// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <unordered_map>
#include <vector>

#include "tcob/core/Property.hpp"
#include "tcob/core/Rect.hpp"
#include "tcob/core/input/Input.hpp"
#include "tcob/gfx/ui/Scrollbar.hpp"
#include "tcob/gfx/ui/Style.hpp"
#include "tcob/gfx/ui/UI.hpp"
#include "tcob/gfx/ui/widgets/Widget.hpp"

namespace tcob::ui {
////////////////////////////////////////////////////////////

// TODO: datasource
class TCOB_API drop_down_list : public widget {
public:
    class TCOB_API style : public widget_style {
    public:
        text_element Text;

        utf8_string NavArrowClass {"nav_arrows"};

        utf8_string ItemClass {"list_items"};
        length      ItemHeight {};

        scrollbar_element VScrollBar;

        void static Transition(style& target, style const& left, style const& right, f64 step);
    };

    explicit drop_down_list(init const& wi);

    prop_val<isize> SelectedItemIndex;
    prop_val<isize> HoveredItemIndex;

    prop<isize> MaxVisibleItems; // TODO: change to prop_val

    void add_item(utf8_string const& item);
    void add_item(list_item const& item);
    void clear_items();

    auto select_item(utf8_string const& item) -> bool;

    auto get_item_at(isize index) const -> list_item const&;
    auto selected_item() const -> list_item const&;
    auto item_count() const -> isize;

protected:
    void on_styles_changed() override;

    void on_draw(widget_painter& painter) final;

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
    auto get_items() const -> std::vector<list_item> const&;

    auto get_item_height() const -> f32;

    void set_extended(bool v);

    auto get_scroll_distance() const -> f32;
    auto get_scroll_max() const -> f32;

    std::vector<list_item>            _items;
    std::unordered_map<isize, rect_f> _itemRectCache;
    isize                             _visibleItems {0};

    bool _isExtended {false};
    bool _mouseOverBox {false};

    scrollbar _vScrollbar;

    drop_down_list::style _style;
};
}
