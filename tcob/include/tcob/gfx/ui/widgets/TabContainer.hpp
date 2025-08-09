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

namespace tcob::ui {
////////////////////////////////////////////////////////////

// TODO:
// enum class tab_header_overflow : u8 {
//    Wrap,
//    Scroll
//};

class TCOB_API tab_container : public widget_container {
public:
    enum class header_mode : u8 {
        Fill,
        Compact
    };

    class TCOB_API style : public widget_style {
    public:
        utf8_string TabItemClass {"tab_items"};
        position    HeaderPosition {position::Top};
        length      HeaderSize;
        header_mode HeaderMode {header_mode::Fill};
        // header_overflow HeaderOverflow {tab_header_mode::Wrap};

        void static Transition(style& target, style const& left, style const& right, f64 step);
    };

    explicit tab_container(init const& wi);

    prop_val<isize> ActiveTabIndex;
    prop_val<isize> HoveredTabIndex;

    prop<isize> MaxTabsPerLine; // TODO: change to prop_val & overflow mode (Wrap | Scroll)

    template <std::derived_from<widget_container> T>
    auto create_tab(utf8_string const& name) -> std::shared_ptr<T>;
    template <std::derived_from<widget_container> T>
    auto create_tab(utf8_string const& name, item const& label) -> std::shared_ptr<T>;

    void remove_tab(widget* tab);
    void clear_tabs();

    void change_tab_label(widget* tab, utf8_string const& label);
    void change_tab_label(widget* tab, item const& label);

    auto find_child_at(point_i pos) -> std::shared_ptr<widget> override;

    auto widgets() const -> std::vector<std::shared_ptr<widget>> const& override;

protected:
    void on_prepare_redraw() override;

    void on_draw(widget_painter& painter) override;
    void on_draw_children(widget_painter& painter) override;

    void on_mouse_leave() override;
    void on_mouse_hover(input::mouse::motion_event const& ev) override;
    void on_mouse_button_down(input::mouse::button_event const& ev) override;

    void on_update(milliseconds deltaTime) override;
    void on_animation_step(string const& val) override;

    void offset_content(rect_f& bounds, bool isHitTest) const override;

    auto attributes() const -> widget_attributes override;

private:
    void offset_tab_content(rect_f& bounds, style const& style) const;
    auto get_tab_line_count() const -> isize;

    std::vector<std::shared_ptr<widget>> _tabs;
    std::vector<item>                    _tabLabels;
    std::vector<rect_f>                  _tabRectCache;

    tab_container::style _style;
};

}

#include "TabContainer.inl"
