// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <unordered_map>
#include <vector>

#include "tcob/core/Property.hpp"
#include "tcob/core/Rect.hpp"
#include "tcob/core/Signal.hpp"
#include "tcob/core/input/Input.hpp"
#include "tcob/gfx/ui/UI.hpp"
#include "tcob/gfx/ui/WidgetPainter.hpp"
#include "tcob/gfx/ui/component/Item.hpp"
#include "tcob/gfx/ui/widgets/VScrollWidget.hpp"

namespace tcob::ui {

////////////////////////////////////////////////////////////

class TCOB_API tree_view : public vscroll_widget {
public:
    class TCOB_API style : public vscroll_widget::style {
    public:
        std::vector<utf8_string> ItemClasses {"tree_items"};
        utf8_string              NavArrowClass {"tree_nav_arrows"};

        f32    MaxVisibleRows {10};
        length IndentSize {16, length::type::Absolute};

        static void Transition(style& target, style const& from, style const& to, f64 step);
    };

    class TCOB_API node {
    public:
        item              Item {};
        std::vector<node> Children {};
        bool              Expanded {false};

        auto operator==(node const& other) const -> bool = default;
    };

    explicit tree_view(init const& wi);

    signal<node*> ItemClick; // TODO: change to *_event

    prop<std::vector<node>> Nodes;

protected:
    void on_draw(widget_painter& painter) override;
    void on_prepare_redraw() override;

    void on_mouse_leave() override;
    void on_mouse_hover(input::mouse::motion_event const& ev) override;
    void on_mouse_button_down(input::mouse::button_event const& ev) override;

    auto get_scroll_max_offset() const -> f32 override;
    auto get_scroll_step() const -> f32 override;

private:
    auto get_row_height(f32 ref) const -> f32;
    auto count_visible(std::vector<node> const& nodes) const -> i32;

    std::unordered_map<node*, rect_f> _rowRectCache;
    node*                             _hoveredNode {nullptr};
    node*                             _selectedNode {nullptr};
    isize                             _visibleRows {0};
    tree_view::style                  _style;
};

}
