// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/ui/widgets/TreeView.hpp"

#include <algorithm>
#include <tuple>
#include <vector>

#include "tcob/core/Common.hpp"
#include "tcob/core/Rect.hpp"
#include "tcob/core/input/Input.hpp"
#include "tcob/gfx/ui/Style.hpp"
#include "tcob/gfx/ui/UI.hpp"
#include "tcob/gfx/ui/WidgetPainter.hpp"

namespace tcob::ui {

void tree_view::style::Transition(style& target, style const& from, style const& to, f64 step)
{
    vscroll_widget::style::Transition(target, from, to, step);
    target.MaxVisibleRows = helper::lerp(from.MaxVisibleRows, to.MaxVisibleRows, step);
    target.IndentSize     = helper::lerp(from.IndentSize, to.IndentSize, step);
}

tree_view::tree_view(init const& wi)
    : vscroll_widget {wi}
{
    Nodes.Changed.connect([this](auto const&) {
        _hoveredNode  = nullptr;
        _selectedNode = nullptr;
        set_scrollbar_value(0);
        clear_sub_styles();
        queue_redraw();
    });

    Class("tree_view");
}

void tree_view::on_prepare_redraw()
{
    prepare_style(_style);
    vscroll_widget::on_prepare_redraw();
}

void tree_view::on_draw(widget_painter& painter)
{
    rect_f rect {draw_background(_style, painter)};

    // scrollbar
    draw_scrollbar(painter, rect);

    // content
    scoped_scissor const guard {painter, this};

    f32 const rowHeight {get_row_height(rect.height())};
    f32 const scrollOffset {scrollbar_offset()};
    f32 const indentSize {_style.IndentSize.calc(rect.width())};

    _visibleRows = static_cast<isize>(rect.height() / rowHeight);
    _rowRectCache.clear();

    auto const draw_nodes {[&, that = this /*fixed in gcc-16*/](this auto const& self, std::vector<node>& nodes, bool draw, i32 depth, i32& row, i32& idx) -> void {
        for (auto& n : nodes) {
            bool const         selected {&n == that->_selectedNode};
            bool const         hovered {&n == that->_hoveredNode};
            widget_flags const flg {.Active = selected, .Hover = hovered};

            if (draw) {
                rect_f rowRect {rect};
                rowRect.Position.Y += (rowHeight * static_cast<f32>(row++)) - scrollOffset;
                rowRect.Size.Height = rowHeight;

                f32 const indent {indentSize * static_cast<f32>(depth)};
                rowRect.Position.X += indent;
                rowRect.Size.Width = rowRect.width() - indent;

                if (rowRect.bottom() >= rect.top() && rowRect.top() <= rect.bottom()) {
                    item_style rowStyle {};
                    prepare_sub_style(rowStyle, idx, that->_style.ItemClass, flg);
                    painter.draw_item(rowStyle.Item, rowRect, n.Item);

                    if (!n.Children.empty()) {
                        nav_arrows_style arrowStyle {};
                        prepare_sub_style(arrowStyle, idx + 1, that->_style.NavArrowClass, flg);
                        std::ignore = painter.draw_nav_arrow(arrowStyle.NavArrow, rowRect,
                                                             n.Expanded ? direction::Down : direction::Right);
                    }
                }
                that->_rowRectCache[&n] = rowRect;
            } else {
                reset_sub_style(idx, that->_style.ItemClass, flg);
                reset_sub_style(idx + 1, that->_style.NavArrowClass, flg);
            }

            idx += 2;
            if (!n.Children.empty()) {
                self(n.Children, draw && n.Expanded, depth + 1, row, idx);
            }
        }
    }};

    Nodes.mutate([&](auto& nodes) {
        i32 row {0};
        i32 idx {0};
        draw_nodes(nodes, true, 0, row, idx);
        return false;
    });
}

void tree_view::on_mouse_leave()
{
    vscroll_widget::on_mouse_leave();

    if (_hoveredNode) {
        _hoveredNode = nullptr;
        queue_redraw();
    }
}

void tree_view::on_mouse_hover(input::mouse::motion_event const& ev)
{
    vscroll_widget::on_mouse_hover(ev);

    auto const mp {screen_to_local(*this, ev.Position)};
    _hoveredNode = nullptr;
    for (auto& [n, r] : _rowRectCache) {
        if (r.contains(mp)) {
            _hoveredNode = n;
            ev.Handled   = true;
            queue_redraw();
            break;
        }
    }
}

void tree_view::on_mouse_button_down(input::mouse::button_event const& ev)
{
    vscroll_widget::on_mouse_button_down(ev);
    if (ev.Button == controls().PrimaryMouseButton && _hoveredNode) {
        _selectedNode = _hoveredNode;

        if (!_selectedNode->Children.empty()) {
            _selectedNode->Expanded = !_selectedNode->Expanded;
        }

        ItemClick(_selectedNode);
        ev.Handled = true;
        queue_redraw();
    }
}

auto tree_view::count_visible(std::vector<node> const& nodes) const -> i32
{
    i32 count {0};
    for (auto const& n : nodes) {
        ++count;
        if (n.Expanded && !n.Children.empty()) {
            count += count_visible(n.Children);
        }
    }
    return count;
}

auto tree_view::get_scroll_max_value() const -> f32
{
    f32 const contentHeight {content_bounds().height()};
    f32 const rowHeight {get_row_height(contentHeight)};
    return std::max(0.0f, (rowHeight * static_cast<f32>(count_visible(Nodes))) - contentHeight);
}

auto tree_view::get_scroll_step() const -> f32
{
    f32 const max {get_scroll_max_value()};
    if (max <= 0.f) { return 1.f; }
    f32 const contentHeight {content_bounds().height()};
    return get_row_height(contentHeight) * static_cast<f32>(_visibleRows) / max;
}

auto tree_view::get_row_height(f32 ref) const -> f32
{
    return ref / _style.MaxVisibleRows;
}
}
