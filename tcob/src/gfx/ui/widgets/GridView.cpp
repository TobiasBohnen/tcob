// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/ui/widgets/GridView.hpp"

#include <algorithm>
#include <iterator>
#include <numeric>
#include <span>
#include <vector>

#include "tcob/core/Point.hpp"
#include "tcob/core/Rect.hpp"
#include "tcob/core/input/Input.hpp"
#include "tcob/gfx/ui/Style.hpp"
#include "tcob/gfx/ui/UI.hpp"
#include "tcob/gfx/ui/WidgetPainter.hpp"
#include "tcob/gfx/ui/widgets/VScrollWidget.hpp"

namespace tcob::ui {

static constexpr point_i INVALID {INVALID_INDEX, INVALID_INDEX};

void grid_view::style::Transition(style& target, style const& left, style const& right, f64 step)
{
    vscroll_widget::style::Transition(target, left, right, step);

    target.RowHeight = length::Lerp(left.RowHeight, right.RowHeight, step);
}

grid_view::grid_view(init const& wi)
    : vscroll_widget {wi}
{
    SelectedCellIndex.Changed.connect([this](auto const&) { request_redraw(this->name() + ": SelectedCell changed"); });
    SelectedCellIndex(INVALID);
    HoveredCellIndex.Changed.connect([this](auto const&) { request_redraw(this->name() + ": HoveredCell changed"); });
    HoveredCellIndex(INVALID);

    SelectMode(select_mode::Cell);
    HeaderSelectable(false);

    Class("grid_view");
}

void grid_view::set_columns(std::vector<utf8_string> const& col, bool clearRows)
{
    if (clearRows) { clear_rows(); }

    _columnHeaders.clear();
    for (auto const& c : col) {
        _columnHeaders.push_back({.Text = c, .Icon = {}, .UserData = {}});
    }

    _columnSizes.resize(_columnHeaders.size());
    for (usize x {0}; x < _columnSizes.size(); ++x) {
        _columnSizes[x] = std::ssize(col[x]);
    }

    request_redraw(this->name() + ": column headers set");
}

auto grid_view::column_count() const -> isize
{
    return std::ssize(_columnHeaders);
}

void grid_view::add_row(std::vector<utf8_string> const& row)
{
    std::vector<list_item> newRow {};
    newRow.resize(_columnHeaders.size());

    auto const size {std::min(row.size(), _columnHeaders.size())};
    for (usize i {0}; i < size; ++i) { newRow[i].Text = row[i]; }

    add_row(newRow);
}

void grid_view::add_row(std::span<list_item const> row)
{
    std::vector<list_item>& newRow {_rows.emplace_back()};
    newRow.resize(_columnHeaders.size());

    auto const size {std::min(row.size(), _columnHeaders.size())};
    for (usize i {0}; i < size; ++i) { newRow[i] = row[i]; }

    for (usize i {0}; i < size; ++i) {
        _columnSizes[i] = std::max(_columnSizes[i], std::ssize(row[i].Text));
    }

    request_redraw(this->name() + ": row added");
}

void grid_view::remove_row(isize idx)
{
    _rows.erase(_rows.begin() + idx);
    // TODO: update _columnSizes

    clear_sub_styles();

    set_scrollbar_value(0);
    SelectedCellIndex = INVALID;
    HoveredCellIndex  = INVALID;

    request_redraw(this->name() + ": row removed");
}

void grid_view::clear_rows()
{
    _rows.clear();
    _columnSizes.clear();
    _columnSizes.resize(_columnHeaders.size());
    clear_sub_styles();

    set_scrollbar_value(0);
    SelectedCellIndex = INVALID;
    HoveredCellIndex  = INVALID;

    request_redraw(this->name() + ": rows cleared");
}

auto grid_view::row_count() const -> isize
{
    return std::ssize(_rows);
}

auto get_cell_impl(point_i idx, auto&& headers, auto&& rows) -> decltype(&headers[0])
{
    if (idx.Y == 0) {
        if (idx.X >= std::ssize(headers)) { return nullptr; }
        return &headers[idx.X];
    }

    if (idx.Y > std::ssize(rows)) { return nullptr; }
    if (idx.X >= std::ssize(rows[idx.Y - 1])) { return nullptr; }
    return &rows[idx.Y - 1][idx.X];
}

auto grid_view::get_cell(point_i idx) -> list_item*
{
    return get_cell_impl(idx, _columnHeaders, _rows);
}

auto grid_view::get_cell(point_i idx) const -> list_item const*
{
    return get_cell_impl(idx, _columnHeaders, _rows);
}

auto grid_view::get_row(isize idx) const -> std::vector<list_item> const&
{
    return _rows[idx];
}

void grid_view::prepare_redraw()
{
    apply_style(_style);
    vscroll_widget::prepare_redraw();
}

void grid_view::on_draw(widget_painter& painter)
{
    rect_f rect {draw_background(_style, painter)};

    // scrollbar
    draw_scrollbar(painter, rect);

    // content
    scissor_guard const guard {painter, this};

    rect_f    gridRect {rect};
    f32 const rowHeight {_style.RowHeight.calc(gridRect.height())};

    std::vector<f32> columnWidths(_columnHeaders.size());
    std::vector<f32> columnOffsets(_columnHeaders.size());

    for (i32 x {0}; x < std::ssize(_columnHeaders); ++x) {
        columnWidths[x] = get_column_width(x, gridRect.width());
        if (x > 0) {
            columnOffsets[x] = columnOffsets[x - 1] + columnWidths[x - 1];
        }
    }

    _rowRectCache.clear();
    _headerRectCache.clear();
    _visibleRows = static_cast<isize>((gridRect.height() / rowHeight) - 1);
    auto const scrollOffset {scrollbar_offset()};

    auto const paintCell {[&](point_i idx, list_item const& item, string const& className, widget_flags cellFlags, rect_f& cell) {
        rect_f cellRect {point_f::Zero, {columnWidths[idx.X], rowHeight}};
        cellRect.Position.X = gridRect.Position.X + columnOffsets[idx.X];
        cellRect.Position.Y = gridRect.Position.Y + (rowHeight * static_cast<f32>(idx.Y));
        if (idx.Y > 0) { cellRect.Position.Y -= scrollOffset; }

        if (cellRect.bottom() > gridRect.top() && cellRect.top() < gridRect.bottom()) {
            item_style cellStyle {};
            apply_sub_style(cellStyle, idx.X + idx.Y * std::ssize(_columnHeaders), className, cellFlags);
            painter.draw_item(cellStyle.Item, cellRect, item);
            cell = cellRect;
        } else {
            reset_sub_style(idx.X + idx.Y * std::ssize(_columnHeaders), className, cellFlags);
        }
    }};

    auto const getCellFlags {[this](point_i idx, select_mode mode) -> widget_flags {
        switch (mode) {
        case select_mode::Cell: return {.Active = idx == SelectedCellIndex, .Hover = idx == HoveredCellIndex};
        case select_mode::Row: return {.Active = idx.Y == SelectedCellIndex->Y, .Hover = idx.Y == HoveredCellIndex->Y};
        case select_mode::Column: return {.Active = idx.X == SelectedCellIndex->X, .Hover = idx.X == HoveredCellIndex->X};
        }
        return {};
    }};

    // Draw rows
    std::vector<point_i> selectedCells;
    std::vector<point_i> hoveredCells;
    for (i32 y {0}; y < std::ssize(_rows); ++y) {
        for (i32 x {0}; x < std::ssize(_columnHeaders); ++x) {
            point_i const idx {x, y + 1};
            auto const    cellFlags {getCellFlags(idx, SelectMode)};
            // skip selected/hover
            if (cellFlags.Active) {
                selectedCells.push_back(idx);
                continue;
            }
            if (cellFlags.Hover) {
                hoveredCells.push_back(idx);
                continue;
            }

            paintCell(idx, _rows[y][x], _style.RowItemClass, cellFlags, _rowRectCache[idx]);
        }
    }

    for (auto const& idx : selectedCells) {
        paintCell(idx, _rows[idx.Y - 1][idx.X], _style.RowItemClass, {.Active = true}, _rowRectCache[idx]);
    }
    for (auto const& idx : hoveredCells) {
        paintCell(idx, _rows[idx.Y - 1][idx.X], _style.RowItemClass, {.Hover = true}, _rowRectCache[idx]);
    }

    // Draw headers
    for (i32 x {0}; x < std::ssize(_columnHeaders); ++x) {
        point_i const idx {x, 0};
        paintCell(idx, _columnHeaders[x], _style.HeaderItemClass, getCellFlags(idx, SelectMode), _headerRectCache[idx]);
    }
}

void grid_view::on_animation_step(string const& val)
{
    if (SelectedCellIndex != INVALID) {
        _rows[SelectedCellIndex->Y - 1][SelectedCellIndex->X].Icon.Region = val;
        request_redraw(this->name() + ": Animation Frame changed ");
    }
}

void grid_view::on_mouse_leave()
{
    vscroll_widget::on_mouse_leave();

    HoveredCellIndex = INVALID;
}

void grid_view::on_mouse_hover(input::mouse::motion_event const& ev)
{
    vscroll_widget::on_mouse_hover(ev);

    auto const mp {global_to_parent(*this, ev.Position)};

    for (auto const& kvp : _headerRectCache) {
        if (!kvp.second.contains(mp)) { continue; }
        if (HeaderSelectable) {
            HoveredCellIndex = kvp.first;
            ev.Handled       = true;
        } else {
            HoveredCellIndex = INVALID;
        }
        return;
    }
    for (auto const& kvp : _rowRectCache) {
        if (!kvp.second.contains(mp)) { continue; }
        HoveredCellIndex = kvp.first;
        ev.Handled       = true;
        return;
    }

    HoveredCellIndex = INVALID;
}

void grid_view::on_mouse_down(input::mouse::button_event const& ev)
{
    vscroll_widget::on_mouse_down(ev);

    if (ev.Button == controls().PrimaryMouseButton) {
        if (HoveredCellIndex != INVALID) {
            if (HeaderSelectable || HoveredCellIndex->Y != 0) {
                SelectedCellIndex = HoveredCellIndex();
            }
        }

        ev.Handled = true;
    }
}

auto grid_view::attributes() const -> widget_attributes
{
    auto retValue {vscroll_widget::attributes()};

    retValue["selected_index"] = SelectedCellIndex();
    if (SelectedCellIndex() != INVALID) {
        retValue["selected"] = get_cell(SelectedCellIndex)->Text;
    }

    retValue["hover_index"] = HoveredCellIndex();
    if (HoveredCellIndex() != INVALID) {
        retValue["hover"] = get_cell(HoveredCellIndex)->Text;
    }

    return retValue;
}

auto grid_view::get_column_width(i32 col, f32 width) const -> f32
{
    if (_style.AutoSizeColumns) {
        auto const sum {std::reduce(_columnSizes.begin(), _columnSizes.end())};
        return width * (static_cast<f32>(_columnSizes[col]) / static_cast<f32>(sum));
    }

    return width / static_cast<f32>(_columnHeaders.size());
}

auto grid_view::get_scroll_content_height() const -> f32
{
    if (_columnHeaders.empty()) { return 0; }

    f32 const itemHeight {_style.RowHeight.calc(content_bounds().height())};
    return itemHeight * static_cast<f32>(_rows.size() + 1);
}

auto grid_view::get_scroll_distance() const -> f32
{
    return _style.RowHeight.calc(content_bounds().height()) * static_cast<f32>(_visibleRows) / get_scroll_max();
}

} // namespace ui
