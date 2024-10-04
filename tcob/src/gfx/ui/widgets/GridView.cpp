// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/ui/widgets/GridView.hpp"

#include <numeric>

#include "tcob/gfx/ui/WidgetPainter.hpp"

namespace tcob::gfx::ui {

grid_view::grid_view(init const& wi)
    : vscroll_widget {wi}
{
    Class("grid_view");
}

void grid_view::set_columns(std::vector<utf8_string> const& col, bool clearRows)
{
    if (clearRows) { clear_rows(); }

    _columnHeaders = col;
    _columnSizes.resize(_columnHeaders.size());
    for (usize x {0}; x < _columnSizes.size(); ++x) {
        _columnSizes[x] = std::ssize(col[x]);
    }

    force_redraw(get_name() + ": column headers set");
}

void grid_view::change_column(isize idx, utf8_string const& col)
{
    if (idx < 0 || idx >= std::ssize(_columnHeaders)) { return; }

    _columnHeaders[idx] = col;
    _columnSizes[idx]   = std::ssize(col);

    force_redraw(get_name() + ": column header changed");
}

void grid_view::add_row(std::vector<utf8_string> const& row)
{
    std::vector<utf8_string> r {row};
    r.resize(_columnHeaders.size());

    _rows.push_back(r);
    if (!_rows.empty()) {
        for (usize x {0}; x < r.size(); ++x) {
            _columnSizes[x] = std::max(_columnSizes[x], std::ssize(r[x]));
        }
    }

    force_redraw(get_name() + ": row added");
}

void grid_view::clear_rows()
{
    _rows.clear();
    _columnSizes.clear();
    _columnSizes.resize(_columnHeaders.size());
    set_scrollbar_value(0);
    force_redraw(get_name() + ": rows cleared");
}

void grid_view::paint_content(widget_painter& painter, rect_f const& rect)
{
    if (auto const* style {get_style<grid_view::style>()}) {
        // content
        scissor_guard const guard {painter, this};

        rect_f gridRect {rect};

        f32 const rowHeight {style->RowHeight.calc(gridRect.Height)};

        std::vector<f32> colWidths(_columnHeaders.size());

        // rows
        for (i32 y {0}; y < std::ssize(_rows); ++y) {
            auto const& row {_rows[y]};
            f32         offsetX {0.f};

            for (i32 x {0}; x < std::ssize(_columnHeaders); ++x) {
                f32 const colWidth {colWidths[x] = get_column_width(style, x, gridRect.Width)};

                rect_f const cellRect {get_cell_rect({x, y + 1}, gridRect.get_position(), {colWidth, rowHeight}, offsetX)};
                if (cellRect.bottom() > 0 && cellRect.top() < gridRect.bottom()) {
                    auto const& cellStyle {get_cell_style({x, y + 1}, style->RowClass)->Item};
                    painter.draw_item(cellStyle, cellRect, row[x]);
                }
                offsetX += colWidth;
            }
        }
        f32 offsetX {0.f};
        for (i32 x {0}; x < std::ssize(_columnHeaders); ++x) {
            // headers
            f32 const colWidth {colWidths[x] = get_column_width(style, x, gridRect.Width)};

            rect_f const cellRect {get_cell_rect({x, 0}, gridRect.get_position(), {colWidth, rowHeight}, offsetX)};
            if (cellRect.bottom() > 0 && cellRect.top() < gridRect.bottom()) {
                auto const& cellStyle {get_cell_style({x, 0}, style->HeaderClass)->Item};
                painter.draw_item(cellStyle, cellRect, _columnHeaders[x]);
            }
            offsetX += colWidth;
        }
    }
}

auto grid_view::get_cell_rect(point_i idx, point_f pos, size_f size, f32 offsetX) const -> rect_f
{
    rect_f retValue {point_f::Zero, size};
    retValue.X = pos.X + offsetX;
    retValue.Y = pos.Y + (size.Height * idx.Y);
    if (idx.Y > 0) {
        retValue.Y -= get_scrollbar_value();
    }

    return retValue;
}

auto grid_view::get_cell_style(point_i /* idx */, string const& className) const -> item_style*
{
    // TODO: implement Hover and Active cell
    return get_sub_style<item_style>(className, {});
}

auto grid_view::get_column_width(grid_view::style const* style, i32 col, f32 width) const -> f32
{
    if (style->AutoSizeColumns) {
        auto const sum {std::reduce(_columnSizes.begin(), _columnSizes.end())};
        return width * (_columnSizes[col] / static_cast<f32>(sum));
    }

    return width / _columnHeaders.size();
}

auto grid_view::get_scroll_content_height() const -> f32
{
    if (_columnHeaders.empty()) { return 0; }

    f32 retValue {0.0f};
    if (auto const* style {get_style<grid_view::style>()}) {
        rect_f const listRect {get_content_bounds()};
        f32 const    itemHeight {style->RowHeight.calc(listRect.Height)};
        retValue += itemHeight * (_rows.size() + 1);
    }

    return retValue;
}

auto grid_view::get_scroll_item_count() const -> isize
{
    return std::ssize(_rows);
}

} // namespace ui
