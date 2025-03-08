// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <memory>
#include <vector>

#include "tcob/core/Point.hpp"
#include "tcob/core/Property.hpp"
#include "tcob/core/Rect.hpp"
#include "tcob/core/input/Input.hpp"
#include "tcob/gfx/ui/Style.hpp"
#include "tcob/gfx/ui/UI.hpp"
#include "tcob/gfx/ui/widgets/Widget.hpp"
#include "tcob/gfx/ui/widgets/WidgetContainer.hpp"

namespace tcob::gfx::ui {
////////////////////////////////////////////////////////////

// TODO: scrollable tab bar
class TCOB_API tab_container : public widget_container {
public:
    enum class position : u8 {
        Top,
        Bottom,
        Hidden
    };

    class TCOB_API style : public widget_style {
    public:
        utf8_string TabItemClass {"tab_items"};
        position    TabBarPosition {position::Top};
        length      TabBarHeight;

        void static Transition(style& target, style const& left, style const& right, f64 step);
    };

    explicit tab_container(init const& wi);

    prop_val<isize> ActiveTabIndex;
    prop_val<isize> HoveredTabIndex;

    prop<isize> MaxTabsPerRow; // TODO: change to prop_val

    template <std::derived_from<widget_container> T>
    auto create_tab(utf8_string const& name) -> std::shared_ptr<T>;
    template <std::derived_from<widget_container> T>
    auto create_tab(utf8_string const& name, list_item const& label) -> std::shared_ptr<T>;

    void prepare_redraw() override;

    void remove_tab(widget* tab);
    void clear_tabs();

    void change_tab_label(widget* tab, utf8_string const& label);

    auto find_child_at(point_f pos) -> std::shared_ptr<widget> override;

    auto widgets() const -> std::vector<std::shared_ptr<widget>> const& override;

protected:
    void on_paint(widget_painter& painter) override;

    void on_mouse_leave() override;
    void on_mouse_hover(input::mouse::motion_event const& ev) override;
    void on_mouse_down(input::mouse::button_event const& ev) override;

    void on_update(milliseconds deltaTime) override;

    void offset_content(rect_f& bounds, bool isHitTest) const override;

    auto attributes() const -> widget_attributes override;

private:
    void offset_tab_content(rect_f& bounds, style const& style) const;
    auto get_tab_row_count() const -> isize;

    std::vector<std::shared_ptr<widget>> _tabs;
    std::vector<list_item>               _tabLabels;
    std::vector<rect_f>                  _tabRectCache;

    tab_container::style _style;

    bool _updateTabs {false};
};

}

#include "TabContainer.inl"
