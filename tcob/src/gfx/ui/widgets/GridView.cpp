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

auto grid_view::column_size() const -> isize
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
    set_scrollbar_value(0);
    SelectedCellIndex = INVALID;
    HoveredCellIndex  = INVALID;

    force_redraw(this->name() + ": rows cleared");
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

void grid_view::paint_content(widget_painter& painter, rect_f const& rect)
{
    // content
    scissor_guard const guard {painter, this};

    rect_f gridRect {rect};

    f32 const rowHeight {_style.RowHeight.calc(gridRect.height())};

    std::vector<f32> colWidths(_columnHeaders.size());

    // rows
    _rowRectCache.clear();
    for (i32 y {0}; y < std::ssize(_rows); ++y) {
        auto const& row {_rows[y]};
        f32         offsetX {0.f};

        for (i32 x {0}; x < std::ssize(_columnHeaders); ++x) {
            f32 const colWidth {colWidths[x] = get_column_width(x, gridRect.width())};

            rect_f const cellRect {get_cell_rect({x, y + 1}, gridRect.Position, {colWidth, rowHeight}, offsetX)};
            if (cellRect.bottom() > 0 && cellRect.top() < gridRect.bottom()) {
                auto const& cellStyle {get_cell_style({x, y + 1}, _style.RowItemClass, SelectMode)->Item};
                painter.draw_item(cellStyle, cellRect, row[x]);
                _rowRectCache[{x, y + 1}] = cellRect;
            }
            offsetX += colWidth;
        }
    }
    f32 offsetX {0.f};
    // headers
    _headerRectCache.clear();
    for (i32 x {0}; x < std::ssize(_columnHeaders); ++x) {
        f32 const colWidth {colWidths[x]};

        rect_f const cellRect {get_cell_rect({x, 0}, gridRect.Position, {colWidth, rowHeight}, offsetX)};
        if (cellRect.bottom() > 0 && cellRect.top() < gridRect.bottom()) {
            auto const& cellStyle {get_cell_style({x, 0}, _style.HeaderItemClass, SelectMode)->Item};
            painter.draw_item(cellStyle, cellRect, _columnHeaders[x]);
            _headerRectCache[{x, 0}] = cellRect;
        }
        offsetX += colWidth;
    }
}

void grid_view::on_mouse_leave()
{
    vscroll_widget::on_mouse_leave();

    HoveredCellIndex = INVALID;
}

void grid_view::on_mouse_hover(input::mouse::motion_event const& ev)
{
    HoveredCellIndex = INVALID;

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

auto grid_view::get_cell_rect(point_i idx, point_f pos, size_f size, f32 offsetX) const -> rect_f
{
    rect_f retValue {point_f::Zero, size};
    retValue.Position.X = pos.X + offsetX;
    retValue.Position.Y = pos.Y + (size.Height * idx.Y);
    if (idx.Y > 0) {
        retValue.Position.Y -= get_scrollbar_value();
    }

    return retValue;
}

auto grid_view::get_cell_style(point_i idx, string const& className, select_mode mode) const -> item_style*
{
    switch (mode) {
    case select_mode::Cell:
        return idx == SelectedCellIndex ? get_sub_style<item_style>(className, {.Active = true})
            : idx == HoveredCellIndex   ? get_sub_style<item_style>(className, {.Hover = true})
                                        : get_sub_style<item_style>(className, {});
    case select_mode::Row:
        return idx.Y == SelectedCellIndex->Y ? get_sub_style<item_style>(className, {.Active = true})
            : idx.Y == HoveredCellIndex->Y   ? get_sub_style<item_style>(className, {.Hover = true})
                                             : get_sub_style<item_style>(className, {});
    case select_mode::Column:
        return idx.X == SelectedCellIndex->X ? get_sub_style<item_style>(className, {.Active = true})
            : idx.X == HoveredCellIndex->X   ? get_sub_style<item_style>(className, {.Hover = true})
                                             : get_sub_style<item_style>(className, {});
    }

    return nullptr;
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

    f32          retValue {0.0f};
    rect_f const listRect {content_bounds()};
    f32 const    itemHeight {_style.RowHeight.calc(listRect.height())};
    retValue += itemHeight * (_rows.size() + 1);

    return retValue;
}

auto grid_view::get_scroll_item_count() const -> isize
{
    return std::ssize(_rows);
}

auto grid_view::update_style() -> vscroll_widget::style*
{
    get_style(_style);
    return &_style;
}

} // namespace ui
