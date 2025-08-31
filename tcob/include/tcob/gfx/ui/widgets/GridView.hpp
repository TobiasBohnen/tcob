// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <unordered_map>
#include <vector>

#include "tcob/core/Grid.hpp"
#include "tcob/core/Point.hpp"
#include "tcob/core/Property.hpp"
#include "tcob/core/Rect.hpp"
#include "tcob/core/input/Input.hpp"
#include "tcob/gfx/ui/UI.hpp"
#include "tcob/gfx/ui/WidgetPainter.hpp"
#include "tcob/gfx/ui/component/Item.hpp"
#include "tcob/gfx/ui/widgets/VScrollWidget.hpp"

namespace tcob::ui {
////////////////////////////////////////////////////////////

// TODO: horizontal scrolling
// TODO: sort by column
class TCOB_API grid_view : public vscroll_widget {
public:
    enum class select_mode : u8 {
        None,
        Cell,
        Row,
        Column
    };

    class TCOB_API style : public vscroll_widget::style {
    public:
        bool                     AutoSizeColumns {true};
        utf8_string              HeaderItemClass {"header_items"};
        std::vector<utf8_string> RowItemClasses {"row_items"};
        f32                      MaxVisibleRows {5};

        static void Transition(style& target, style const& from, style const& to, f64 step);
    };

    explicit grid_view(init const& wi);

    prop<std::vector<item>> Header;
    prop<grid<item>>        Grid;

    prop<point_i>     SelectedCellIndex; // TODO: change to prop_val
    prop<point_i>     HoveredCellIndex;  // TODO: change to prop_val
    prop<select_mode> SelectMode;
    prop<bool>        HeaderSelectable;

    auto get_cell(point_i idx) const -> item const&;

protected:
    void on_draw(widget_painter& painter) override;

    void on_prepare_redraw() override;

    void on_mouse_leave() override;
    void on_mouse_hover(input::mouse::motion_event const& ev) override;
    void on_mouse_button_down(input::mouse::button_event const& ev) override;

    auto attributes() const -> widget_attributes override;

    auto get_scroll_max_value() const -> f32 override;
    auto get_scroll_step() const -> f32 override;

private:
    auto get_column_width(usize col, f32 width) const -> f32;
    auto get_row_height(f32 ref) const -> f32;

    std::vector<isize> _columnSizes;

    std::unordered_map<point_i, rect_f> _headerRectCache;
    std::unordered_map<point_i, rect_f> _rowRectCache;
    isize                               _visibleRows {0};

    grid_view::style _style;
};
}
