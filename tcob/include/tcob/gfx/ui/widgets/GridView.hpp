// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <span>
#include <unordered_map>
#include <vector>

#include "tcob/core/Point.hpp"
#include "tcob/core/Property.hpp"
#include "tcob/core/Rect.hpp"
#include "tcob/core/input/Input.hpp"
#include "tcob/gfx/ui/UI.hpp"
#include "tcob/gfx/ui/WidgetPainter.hpp"
#include "tcob/gfx/ui/widgets/VScrollWidget.hpp"

namespace tcob::ui {
////////////////////////////////////////////////////////////

// TODO: horizontal scrolling
// TODO: datasource
// TODO: sort by column
class TCOB_API grid_view : public vscroll_widget {
public:
    enum class select_mode : u8 {
        Cell,
        Row,
        Column
    };

    class TCOB_API style : public vscroll_widget::style {
    public:
        bool        AutoSizeColumns {true};
        utf8_string HeaderItemClass {"header_items"};
        utf8_string RowItemClass {"row_items"};
        length      RowHeight {};

        void static Transition(style& target, style const& left, style const& right, f64 step);
    };

    explicit grid_view(init const& wi);

    prop<point_i>     SelectedCellIndex; // TODO: change to prop_val
    prop<point_i>     HoveredCellIndex;  // TODO: change to prop_val
    prop<select_mode> SelectMode;
    prop<bool>        HeaderSelectable;

    void set_columns(std::vector<utf8_string> const& col, bool clearRows = true);
    auto column_count() const -> isize;

    void add_row(std::vector<utf8_string> const& row);
    void add_row(std::span<list_item const> row);
    void remove_row(isize idx);
    void clear_rows();
    auto row_count() const -> isize;

    auto get_cell(point_i idx) const -> utf8_string;
    auto get_row(isize idx) const -> std::vector<list_item> const&;

    void prepare_redraw() override;

protected:
    void on_paint(widget_painter& painter) override;

    void on_mouse_leave() override;
    void on_mouse_hover(input::mouse::motion_event const& ev) override;
    void on_mouse_down(input::mouse::button_event const& ev) override;

    auto attributes() const -> widget_attributes override;

    auto get_scroll_content_height() const -> f32 override;
    auto get_scroll_distance() const -> f32 override;

private:
    auto get_column_width(i32 col, f32 width) const -> f32;

    std::vector<list_item>              _columnHeaders;
    std::vector<isize>                  _columnSizes;
    std::vector<std::vector<list_item>> _rows;

    std::unordered_map<point_i, rect_f> _headerRectCache;
    std::unordered_map<point_i, rect_f> _rowRectCache;
    isize                               _visibleRows {0};

    grid_view::style _style;
};
}
