// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/ui/widgets/GridView.hpp"

#include <numeric>

#include "tcob/gfx/ui/Form.hpp"
#include "tcob/gfx/ui/WidgetPainter.hpp"

namespace tcob::gfx::ui {

static constexpr point_i INVALID {INVALID_INDEX, INVALID_INDEX};

void grid_view::style::Transition(style& target, style const& left, style const& right, f64 step)
{
    vscroll_widget::style::Transition(target, left, right, step);

    target.RowHeight = length::Lerp(left.RowHeight, right.RowHeight, step);
}

grid_view::grid_view(init const& wi)
    : vscroll_widget {wi}
{
    SelectedCellIndex.Changed.connect([this](auto const&) { force_redraw(this->name() + ": SelectedCell changed"); });
    SelectedCellIndex(INVALID);
    HoveredCellIndex.Changed.connect([this](auto const&) { force_redraw(this->name() + ": HoveredCell changed"); });
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

    force_redraw(this->name() + ": column headers set");
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

    force_redraw(this->name() + ": row added");
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

    force_redraw(this->name() + ": rows cleared");
}

auto grid_view::row_count() const -> isize
{
    return std::ssize(_rows);
}

auto grid_view::get_cell(point_i idx) const -> utf8_string
{
    if (idx.Y == 0) {
        if (idx.X >= std::ssize(_columnHeaders)) { return ""; }
        return _columnHeaders[idx.X].Text;
    }

    if (idx.Y > std::ssize(_rows)) { return ""; }
    if (idx.X >= std::ssize(_rows[idx.Y - 1])) { return ""; }
    return _rows[idx.Y - 1][idx.X].Text;
}

void grid_view::prepare_redraw()
{
    update_style(_style);
    vscroll_widget::prepare_redraw();
}

void grid_view::on_paint(widget_painter& painter)
{
    update_style(_style);

    rect_f rect {Bounds()};

    // background
    painter.draw_background_and_border(_style, rect, false);

    // scrollbar
    paint_scrollbar(painter, rect);

    // content
    scissor_guard const guard {painter, this};

    auto const get_cell_rect {[this](point_i idx, point_f pos, size_f size, f32 offsetX) {
        rect_f retValue {point_f::Zero, size};
        retValue.Position.X = pos.X + offsetX;
        retValue.Position.Y = pos.Y + (size.Height * idx.Y);
        if (idx.Y > 0) {
            retValue.Position.Y -= get_scrollbar_value();
        }

        return retValue;
    }};

    auto const get_cell_flags {[this](point_i idx, select_mode mode) -> widget_flags {
        switch (mode) {
        case select_mode::Cell:
            return {.Active = idx == SelectedCellIndex, .Hover = idx == HoveredCellIndex};
        case select_mode::Row:
            return {.Active = idx.Y == SelectedCellIndex->Y, .Hover = idx.Y == HoveredCellIndex->Y};
        case select_mode::Column:
        default:
            return {.Active = idx.X == SelectedCellIndex->X, .Hover = idx.X == HoveredCellIndex->X};
        }
    }};

    rect_f           gridRect {rect};
    f32 const        rowHeight = _style.RowHeight.calc(gridRect.height());
    std::vector<f32> colWidths(_columnHeaders.size());

    for (i32 x {0}; x < std::ssize(_columnHeaders); ++x) {
        colWidths[x] = get_column_width(x, gridRect.width());
    }

    _rowRectCache.clear();
    _headerRectCache.clear();
    _visibleRows = (gridRect.height() / rowHeight) - 1;

    auto const paint_cell {[&](point_i idx, f32 offsetX, list_item const& item, string const& className, rect_f& cell) {
        rect_f const cellRect {get_cell_rect(idx, gridRect.Position, {colWidths[idx.X], rowHeight}, offsetX)};

        if (cellRect.bottom() > gridRect.top() && cellRect.top() < gridRect.bottom()) {
            item_style cellStyle {};
            update_sub_style(cellStyle, idx.X + idx.Y * std::ssize(_columnHeaders), className, get_cell_flags(idx, SelectMode));

            painter.draw_item(cellStyle.Item, cellRect, item);
            cell = cellRect;
            return;
        }

        reset_sub_style(idx.X + idx.Y * std::ssize(_columnHeaders), className, get_cell_flags(idx, SelectMode));
    }};

    // Draw rows
    for (i32 y {0}; y < std::ssize(_rows); ++y) {
        f32 offsetX {0.f};

        for (i32 x {0}; x < std::ssize(_columnHeaders); ++x) {
            point_i const idx {x, y + 1};
            paint_cell(idx, offsetX, _rows[y][x], _style.RowItemClass, _rowRectCache[idx]);
            offsetX += colWidths[x];
        }
    }

    // Draw headers
    f32 offsetX {0.f};
    for (i32 x {0}; x < std::ssize(_columnHeaders); ++x) {
        point_i const idx {x, 0};
        paint_cell(idx, offsetX, _columnHeaders[x], _style.HeaderItemClass, _headerRectCache[idx]);
        offsetX += colWidths[x];
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

    auto const mp {global_to_local(ev.Position)};

    for (auto const& kvp : _headerRectCache) {
        if (!kvp.second.contains(mp)) { continue; }
        if (HeaderSelectable) {
            HoveredCellIndex = kvp.first;
            ev.Handled       = true;
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

    if (ev.Button == parent_form()->Controls->PrimaryMouseButton) {
        force_redraw(this->name() + ": mouse down");

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
    if (SelectedCellIndex() != INVALID) {
        retValue["selected"] = get_cell(SelectedCellIndex);
    }
    if (HoveredCellIndex() != INVALID) {
        retValue["hover"] = get_cell(HoveredCellIndex);
    }
    return retValue;
}

auto grid_view::get_column_width(i32 col, f32 width) const -> f32
{
    if (_style.AutoSizeColumns) {
        auto const sum {std::reduce(_columnSizes.begin(), _columnSizes.end())};
        return width * (_columnSizes[col] / static_cast<f32>(sum));
    }

    return width / _columnHeaders.size();
}

auto grid_view::get_scroll_content_height() const -> f32
{
    if (_columnHeaders.empty()) { return 0; }

    f32 const itemHeight {_style.RowHeight.calc(content_bounds().height())};
    return itemHeight * (_rows.size() + 1);
}

auto grid_view::get_scroll_distance() const -> f32
{
    return _style.RowHeight.calc(content_bounds().height()) * _visibleRows;
}

} // namespace ui
