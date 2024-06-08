// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <vector>

#include "tcob/gfx/ui/widgets/WidgetContainer.hpp"

namespace tcob::gfx::ui {
////////////////////////////////////////////////////////////

// TODO: scrollable tab bar
class TCOB_API tab_container : public widget_container {
public:
    enum class position {
        Top,
        Bottom
    };

    class TCOB_API style : public background_style {
    public:
        string   TabItemClass {"tab_items"};
        position TabBarPosition {position::Top};
        length   TabBarHeight;
    };

    explicit tab_container(init const& wi);

    prop_val<isize> ActiveTabIndex;
    prop_val<isize> HoveredTabIndex;
    prop<isize>     MaxTabs; // TODO: move to style

    template <std::derived_from<widget_container> T>
    auto create_tab(utf8_string const& name) -> std::shared_ptr<T>;
    template <std::derived_from<widget_container> T>
    auto create_tab(utf8_string const& name, utf8_string const& label) -> std::shared_ptr<T>;

    void remove_tab(widget* tab);
    void clear_tabs();

    auto find_child_at(point_f pos) -> std::shared_ptr<widget> override;

    auto get_widgets() const -> std::vector<std::shared_ptr<widget>> const& override;

    void force_redraw(string const& reason) override;

protected:
    void on_styles_changed() override;

    void on_paint(widget_painter& painter) override;

    void on_mouse_leave() override;
    void on_mouse_hover(input::mouse::motion_event& ev) override;
    void on_mouse_down(input::mouse::button_event& ev) override;

    void on_update(milliseconds deltaTime) override;

    void offset_content(rect_f& bounds, bool isHitTest) const override;

private:
    auto get_tab_rect(isize index, rect_f const& rect) const -> rect_f;
    auto get_tab_style(isize index) const -> item_style*;

    void offset_tab_content(rect_f& bounds, style const& style) const;

    void update_tab_bounds();

    std::vector<std::shared_ptr<widget>> _tabs;
    std::vector<utf8_string>             _tabLabels;
    std::vector<rect_f>                  _tabRects;
    bool                                 _isDirty {true};
};

}

#include "TabContainer.inl"
