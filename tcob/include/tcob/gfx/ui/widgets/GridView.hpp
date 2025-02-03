// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include "tcob/gfx/ui/widgets/VScrollWidget.hpp"

namespace tcob::gfx::ui {
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
        utf8_string HeaderClass {"header_items"};
        utf8_string RowClass {"row_items"};
        length      RowHeight {};
        select_mode SelectMode {select_mode::Cell};
    };

    explicit grid_view(init const& wi);

    prop<point_i> SelectedCellIndex; // TODO: change to prop_val
    prop<point_i> HoveredCellIndex;  // TODO: change to prop_val
    prop<bool>    HeaderSelectable;

    void set_columns(std::vector<utf8_string> const& col, bool clearRows = true);
    auto column_size() const -> isize;

    void add_row(std::vector<utf8_string> const& row);
    void add_row(std::span<list_item const> row);
    void clear_rows();

    auto get_cell(point_i idx) const -> utf8_string;

protected:
    void paint_content(widget_painter& painter, rect_f const& rect) override;

    void on_mouse_leave() override;
    void on_mouse_hover(input::mouse::motion_event const& ev) override;
    void on_mouse_down(input::mouse::button_event const& ev) override;

    auto get_scroll_content_height() const -> f32 override;
    auto get_scroll_item_count() const -> isize override;

private:
    auto get_cell_rect(point_i idx, point_f pos, size_f size, f32 offsetX) const -> rect_f;
    auto get_cell_style(point_i idx, string const& className, select_mode mode) const -> item_style*;
    auto get_column_width(grid_view::style const* style, i32 col, f32 width) const -> f32;

    std::vector<list_item>              _columnHeaders;
    std::vector<isize>                  _columnSizes;
    std::vector<std::vector<list_item>> _rows;

    std::unordered_map<point_i, rect_f> _headerRectCache;
    std::unordered_map<point_i, rect_f> _rowRectCache;
};
}
