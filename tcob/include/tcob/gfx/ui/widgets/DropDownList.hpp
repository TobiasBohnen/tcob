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
#include "tcob/gfx/ui/Style.hpp"
#include "tcob/gfx/ui/StyleElements.hpp"
#include "tcob/gfx/ui/UI.hpp"
#include "tcob/gfx/ui/component/Item.hpp"
#include "tcob/gfx/ui/component/Scrollbar.hpp"
#include "tcob/gfx/ui/widgets/Widget.hpp"

namespace tcob::ui {
////////////////////////////////////////////////////////////

class TCOB_API drop_down_list : public widget {
public:
    class TCOB_API style : public widget_style {
    public:
        text_element Text;

        utf8_string NavArrowClass {"nav_arrows"};

        utf8_string ItemClass {"list_items"};
        length      ItemHeight {};

        scrollbar_element VScrollBar;

        void static Transition(style& target, style const& from, style const& to, f64 step);
    };

    explicit drop_down_list(init const& wi);

    prop_val<isize> SelectedItemIndex;
    prop_val<isize> HoveredItemIndex;

    prop<std::vector<item>> Items;
    prop<isize>             MaxVisibleItems; // TODO: change to prop_val

    auto select_item(utf8_string const& item) -> bool;

    auto selected_item() const -> item const&;

protected:
    void on_styles_changed() override;

    void on_draw(widget_painter& painter) final;

    void on_mouse_leave() override;
    void on_mouse_hover(input::mouse::motion_event const& ev) override;
    void on_mouse_drag(input::mouse::motion_event const& ev) override;
    void on_mouse_button_down(input::mouse::button_event const& ev) override;
    void on_mouse_button_up(input::mouse::button_event const& ev) override;
    void on_double_click() override;
    void on_mouse_wheel(input::mouse::wheel_event const& ev) override;

    void on_focus_lost() override;

    void on_update(milliseconds deltaTime) override;

    void offset_content(rect_f& bounds, bool isHitTest) const override;

    auto attributes() const -> widget_attributes override;

private:
    auto get_items() const -> std::vector<item> const&;

    auto get_item_height() const -> f32;

    void set_extended(bool v);

    auto get_scroll_distance() const -> f32;
    auto get_scroll_max() const -> f32;

    std::unordered_map<isize, rect_f> _itemRectCache;
    rect_f                            _chevronRectCache;
    isize                             _visibleItems {0};

    bool _isExtended {false};
    bool _mouseOverBox {false};
    bool _mouseOverChevron {false};

    scrollbar _vScrollbar;

    drop_down_list::style _style;
};
}
