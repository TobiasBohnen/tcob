// Copyright (c) 2023 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include "tcob/gfx/ui/widgets/VScrollWidget.hpp"

namespace tcob::gfx::ui {
////////////////////////////////////////////////////////////

class TCOB_API grid_view : public vscroll_widget {
public:
    class TCOB_API style : public vscroll_widget::style {
    public:
        length RowHeight {};
        string HeaderClass {"header_items"};
        string RowClass {"row_items"};
    };

    explicit grid_view(init const& wi);

    void set_columns(std::vector<utf8_string> const& col);
    void add_row(std::vector<utf8_string> const& row);
    void clear_rows();

protected:
    void paint_content(widget_painter& painter, rect_f const& rect) override;

    auto get_list_height() const -> f32 override;
    auto get_list_item_count() const -> isize override;

private:
    auto get_cell_rect(point_i idx, point_f pos, size_f size, f32 offsetX) const -> rect_f;
    auto get_cell_style(point_i idx, string const& className) const -> item_style*;
    auto get_column_width(i32 col, f32 width) const -> f32;

    std::vector<utf8_string>              _columns;
    std::vector<std::vector<utf8_string>> _rows;
    std::vector<isize>                    _rowSizes;
};
}
