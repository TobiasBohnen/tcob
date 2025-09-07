// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/ui/Layout.hpp"

#include <algorithm>
#include <cstdlib>
#include <iterator>
#include <memory>
#include <span>
#include <variant>
#include <vector>

#include "tcob/core/AngleUnits.hpp"
#include "tcob/core/Common.hpp"
#include "tcob/core/Point.hpp"
#include "tcob/core/Rect.hpp"
#include "tcob/core/Size.hpp"
#include "tcob/core/StringUtils.hpp"
#include "tcob/gfx/Gfx.hpp"
#include "tcob/gfx/ui/Form.hpp"
#include "tcob/gfx/ui/UI.hpp"
#include "tcob/gfx/ui/widgets/Widget.hpp"
#include "tcob/gfx/ui/widgets/WidgetContainer.hpp"

namespace tcob::ui {

layout::layout(parent parent)
    : _parent {parent}
{
}

void layout::apply(size_f size)
{
    if (_widgets.empty()) { return; }
    normalize_zorder();
    do_layout(size);
}

void layout::remove(widget* target)
{
    for (usize i {0}; i < _widgets.size(); ++i) {
        if (_widgets[i].get() == target) {
            _widgets.erase(_widgets.begin() + i);
            Changed();
            return;
        }
    }
}

void layout::clear()
{
    _widgets.clear();
    Changed();
}

auto layout::widgets() const -> std::span<std::shared_ptr<widget> const>
{
    return _widgets;
}

void layout::bring_to_front(widget* target)
{
    if (!target || _widgets.empty()) { return; }
    target->ZOrder = _widgets.size() + 1;
    normalize_zorder();
}

void layout::send_to_back(widget* target)
{
    if (!target || _widgets.empty()) { return; }
    target->ZOrder = 0;
    normalize_zorder();
}

void layout::normalize_zorder()
{
    std::ranges::stable_sort(_widgets, [](auto const& a, auto const& b) { return a->ZOrder > b->ZOrder; });

    auto newZ {static_cast<isize>(_widgets.size())};
    for (auto& w : _widgets) { w->ZOrder = newZ--; }
}

auto layout::allows_move() const -> bool
{
    return false;
}

auto layout::allows_resize() const -> bool
{
    return false;
}

auto layout::create_init(string const& name) const -> widget::init
{
    widget::init retValue {};
    if (name.empty()) {
        retValue.Name = helper::random_string(12);
    } else {
        retValue.Name = name;
    }

    std::visit(
        overloaded {
            [this, &retValue](widget_container* parent) {
                retValue.Form   = &parent->form();
                retValue.Parent = parent;
                Changed();
            },
            [this, &retValue](form_base* parent) {
                retValue.Form   = parent;
                retValue.Parent = nullptr;
                Changed();
            }},
        _parent);

    return retValue;
}

////////////////////////////////////////////////////////////

manual_layout::manual_layout(parent parent)
    : layout {parent}
{
}

auto manual_layout::allows_move() const -> bool
{
    return true;
}

auto manual_layout::allows_resize() const -> bool
{
    return true;
}

void manual_layout::do_layout(size_f /* size */)
{
}

////////////////////////////////////////////////////////////

flex_size_layout::flex_size_layout(parent parent)
    : layout {parent}
{
}

auto flex_size_layout::allows_move() const -> bool
{
    return true;
}

void flex_size_layout::do_layout(size_f size)
{
    auto const& w {widgets()};

    for (auto const& widget : w) {
        auto bounds {*widget->Bounds};
        bounds.Size.Width  = widget->Flex->Width.calc(size.Width);
        bounds.Size.Height = widget->Flex->Height.calc(size.Height);
        widget->Bounds     = bounds;
    }
}

////////////////////////////////////////////////////////////

dock_layout::dock_layout(parent parent)
    : layout {parent}
{
}

void dock_layout::do_layout(size_f size)
{
    auto const& w {widgets()};

    rect_f layoutRect {point_f::Zero, size};
    for (auto const& widget : w) {
        if (layoutRect.height() <= 0 || layoutRect.width() <= 0) {
            widget->Bounds = rect_f::Zero;
            continue;
        }

        f32 const width {std::min(layoutRect.width(), widget->Flex->Width.calc(size.Width))};
        f32 const height {std::min(layoutRect.height(), widget->Flex->Height.calc(size.Height))};

        rect_f widgetBounds {layoutRect};

        switch (_widgetDock.at(widget.get())) {
        case dock_style::Left:
            layoutRect.Position.X += width;
            layoutRect.Size.Width -= width;
            break;
        case dock_style::Right:
            widgetBounds.Position.X = widgetBounds.right() - width;
            layoutRect.Size.Width -= width;
            break;
        case dock_style::Top:
            layoutRect.Position.Y += height;
            layoutRect.Size.Height -= height;
            break;
        case dock_style::Bottom:
            widgetBounds.Position.Y = widgetBounds.bottom() - height;
            layoutRect.Size.Height -= height;
            break;
        case dock_style::Fill:
            layoutRect.Position.X += width;
            layoutRect.Size.Width -= width;
            layoutRect.Position.Y += height;
            layoutRect.Size.Height -= height;
            break;
        }

        widgetBounds.Size.Width  = width;
        widgetBounds.Size.Height = height;
        widget->Bounds           = widgetBounds;
    }
}

////////////////////////////////////////////////////////////

grid_layout::grid_layout(parent parent, size_i initSize, bool autoGrow)
    : layout {parent}
    , _grid {initSize}
    , _autoGrow {autoGrow}
{
}

void grid_layout::do_layout(size_f size)
{
    auto const& w {widgets()};

    f32 const horiSize {size.Width / _grid.Width};
    f32 const vertSize {size.Height / _grid.Height};

    for (auto const& widget : w) {
        rect_f bounds {_widgetBounds[widget.get()]};
        bounds.Position.X *= horiSize;
        bounds.Size.Width = widget->Flex->Width.calc(bounds.Size.Width * horiSize);
        bounds.Position.Y *= vertSize;
        bounds.Size.Height = widget->Flex->Height.calc(bounds.Size.Height * vertSize);

        widget->Bounds = bounds;
    }
}

////////////////////////////////////////////////////////////

box_layout::box_layout(parent parent, size_i boxSize)
    : layout {parent}
    , _box {boxSize}
{
}

void box_layout::do_layout(size_f size)
{
    auto const& w {widgets()};

    f32 const horiSize {size.Width / _box.Width};
    f32 const vertSize {size.Height / _box.Height};

    for (i32 i {0}; i < std::ssize(w); ++i) {
        auto const& widget {w[i]};
        if (i < _box.Width * _box.Height) {
            widget->Bounds = {static_cast<f32>(i % _box.Width) * horiSize,
                              static_cast<f32>(i / _box.Width) * vertSize,
                              widget->Flex->Width.calc(horiSize),
                              widget->Flex->Height.calc(vertSize)};
        } else {
            widget->Bounds = rect_f::Zero;
        }
    }
}

////////////////////////////////////////////////////////////

horizontal_layout::horizontal_layout(parent parent, gfx::vertical_alignment alignment)
    : layout {parent}
    , _alignment {alignment}
{
}

void horizontal_layout::do_layout(size_f size)
{
    auto const& w {widgets()};

    std::vector<f32> widths;
    widths.reserve(w.size());

    f32 totalDesiredWidth {0.f};
    for (auto const& widget : w) {
        f32 const width {widget->Flex->Width.calc(size.Width)};
        widths.push_back(width);
        totalDesiredWidth += width;
    }

    f32 const scale {(totalDesiredWidth > size.Width) ? (size.Width / totalDesiredWidth) : 1.f};
    f32 const remaining {size.Width - (totalDesiredWidth * scale)};
    f32 const offsetX {remaining / (w.size() + 1)};

    f32 x {offsetX};
    for (usize i {0}; i < w.size(); ++i) {
        f32 const width {widths[i] * scale};
        f32 const height {w[i]->Flex->Height.calc(size.Height)};

        f32 y {0.f};
        switch (_alignment) {
        case gfx::vertical_alignment::Top:    y = 0.f; break;
        case gfx::vertical_alignment::Bottom: y = size.Height - height; break;
        case gfx::vertical_alignment::Middle: y = (size.Height - height) / 2.f; break;
        }

        w[i]->Bounds = {x, y, width, height};
        x += width + offsetX;
    }
}

////////////////////////////////////////////////////////////

vertical_layout::vertical_layout(parent parent, gfx::horizontal_alignment alignment)
    : layout {parent}
    , _alignment {alignment}
{
}

void vertical_layout::do_layout(size_f size)
{
    auto const& w {widgets()};

    std::vector<f32> heights;
    heights.reserve(w.size());

    f32 totalDesiredHeight {0.f};
    for (auto const& widget : w) {
        f32 const height {widget->Flex->Height.calc(size.Height)};
        heights.push_back(height);
        totalDesiredHeight += height;
    }

    f32 const scale {(totalDesiredHeight > size.Height) ? (size.Height / totalDesiredHeight) : 1.f};
    f32 const remaining {size.Height - (totalDesiredHeight * scale)};
    f32 const offsetY {remaining / (w.size() + 1)};

    f32 y {offsetY};
    for (usize i {0}; i < w.size(); ++i) {
        auto const& widget {w[i]};
        f32 const   height {heights[i] * scale};
        f32 const   width {widget->Flex->Width.calc(size.Width)};

        f32 x {0.f};
        switch (_alignment) {
        case gfx::horizontal_alignment::Left:     x = 0.f; break;
        case gfx::horizontal_alignment::Right:    x = size.Width - width; break;
        case gfx::horizontal_alignment::Centered: x = (size.Width - width) / 2.f; break;
        }

        widget->Bounds = {x, y, width, height};
        y += height + offsetY;
    }
}

////////////////////////////////////////////////////////////

flow_layout::flow_layout(parent parent)
    : layout {parent}
{
}

void flow_layout::do_layout(size_f size)
{
    auto const& w {widgets()};

    f32 const availableWidth {size.Width};
    f32       x {0.0f};
    f32       y {0.0f};
    f32       rowHeight {0.0f};

    for (auto const& widget : w) {
        f32 const widgetWidth {widget->Flex->Width.calc(availableWidth)};
        f32 const widgetHeight {widget->Flex->Height.calc(size.Height)};

        if (x + widgetWidth > availableWidth) {
            x = 0.0f;
            y += rowHeight;
            rowHeight = 0.0f;
        }

        widget->Bounds = {x, y, widgetWidth, widgetHeight};

        x += widgetWidth;
        rowHeight = std::max(rowHeight, widgetHeight);
    }
}

////////////////////////////////////////////////////////////

masonry_layout::masonry_layout(parent parent, i32 columns)
    : layout {parent}
    , _columns {columns}
{
}

void masonry_layout::do_layout(size_f size)
{
    auto const&      w {widgets()};
    f32 const        horiSize {size.Width / _columns};
    std::vector<f32> colHeights(_columns, 0.0f);

    for (auto const& widget : w) {
        f32 const widgetWidth {widget->Flex->Width.calc(horiSize)};
        f32 const widgetHeight {widget->Flex->Height.calc(size.Height)};

        // Find the shortest column that can fit the widget
        i32 colIndex {-1};
        for (i32 i {0}; i < _columns; ++i) {
            if (colHeights[i] + widgetHeight <= size.Height && (colIndex == -1 || colHeights[i] < colHeights[colIndex])) {
                colIndex = i;
            }
        }

        // If no column can fit the widget, skip it
        if (colIndex == -1) {
            widget->Bounds = rect_f::Zero;
            continue;
        }

        // Place the widget in the selected column
        f32 const x {colIndex * horiSize};
        f32 const y {colHeights[colIndex]};
        widget->Bounds = {x, y, widgetWidth, widgetHeight};
        colHeights[colIndex] += widgetHeight;
    }
}

////////////////////////////////////////////////////////////

tree_layout::tree_layout(parent parent)
    : layout {parent}
{
}

void tree_layout::do_layout(size_f size)
{
    auto const& w {widgets()};
    f32 const   horiSize {size.Width / static_cast<f32>(_maxLevel + 1)};
    f32 const   vertSize {size.Height / static_cast<f32>(w.size())};

    f32 y {0.0f};
    for (auto const& widget : w) {
        f32 const widgetWidth {widget->Flex->Width.calc(horiSize)};
        f32 const widgetHeight {widget->Flex->Height.calc(vertSize)};

        i32 const level {_levels[widget.get()]};
        f32 const x {level * horiSize};
        widget->Bounds = {{x, y}, {widgetWidth, widgetHeight}};
        y += vertSize;
    }
}

////////////////////////////////////////////////////////////

stack_layout::stack_layout(parent parent)
    : layout {parent}
{
}

void stack_layout::activate_widget(widget* widget)
{
    _active = widget;
}

void stack_layout::do_layout(size_f size)
{
    auto const& w {widgets()};
    f32 const   horiSize {size.Width};
    f32 const   vertSize {size.Height};

    for (auto const& widget : w) {
        if (widget.get() == _active) {
            rect_f const bounds {point_f::Zero, {widget->Flex->Width.calc(horiSize), widget->Flex->Height.calc(vertSize)}};
            if (widget->Bounds != bounds) {
                widget->Bounds = bounds;
                widget->form().rehover_widget(widget.get());
            }
        } else {
            widget->Bounds = rect_f::Zero;
        }
    }
}

////////////////////////////////////////////////////////////

circle_layout::circle_layout(parent parent, length radius)
    : layout {parent}
    , _radius {radius}
{
}

void circle_layout::do_layout(size_f size)
{
    auto const& w {widgets()};
    f32 const   horiSize {size.Width};
    f32 const   vertSize {size.Height};

    point_f const center {size.Width / 2.0f, size.Height / 2.0f};
    auto const    angleStep {TAU_F / static_cast<f32>(w.size())};
    f32 const     radius {_radius.calc(std::min(horiSize / 2.0f, vertSize / 2.0f))};

    for (usize i {0}; i < w.size(); ++i) {
        f32 const  angle {i * angleStep};
        auto const pos {center + (point_f::FromDirection(radian_f {angle}) * radius)};

        auto const& widget {w[i]};
        f32 const   widgetWidth {widget->Flex->Width.calc(horiSize)};
        f32 const   widgetHeight {widget->Flex->Height.calc(vertSize)};

        widget->Bounds = {{pos.X - (widgetWidth / 2.0f), pos.Y - (widgetHeight / 2.0f)}, {widgetWidth, widgetHeight}};
    }
}

////////////////////////////////////////////////////////////

magnetic_snap_layout::magnetic_snap_layout(parent parent, f32 distance, bool snapEdges, bool snapSiblings)
    : layout {parent}
    , _distance {distance}
    , _snapEdges {snapEdges}
    , _snapSiblings {snapSiblings}
{
}

auto magnetic_snap_layout::allows_move() const -> bool
{
    return true;
}

void magnetic_snap_layout::do_layout(size_f size)
{
    auto const& w {widgets()};

    for (usize i {0}; i < w.size(); ++i) {
        auto const& widget {w[i]};
        auto        b {*widget->Bounds};
        auto const  border {widget->current_style()->Border.Size.calc(b.width())};

        f32 const left {b.left() - border};
        f32 const right {b.right() + border};
        f32 const top {b.top() - border};
        f32 const bottom {b.bottom() + border};

        bool hasSnappedX {false};
        bool hasSnappedY {false};

        if (_snapEdges) {
            if (left <= _distance) {
                b.Position.X = border / 2;
                hasSnappedX  = true;
            } else if (right >= size.Width - _distance) {
                b.Position.X = size.Width - b.width() - (border / 2);
                hasSnappedX  = true;
            }
            if (top <= _distance) {
                b.Position.Y = border / 2;
                hasSnappedY  = true;
            } else if (bottom >= size.Height - _distance) {
                b.Position.Y = size.Height - b.height() - (border / 2);
                hasSnappedY  = true;
            }
        }

        if (_snapSiblings) {
            for (usize j {i + 1}; j < w.size(); ++j) {
                auto const& other {w[j]};
                auto const& o {*other->Bounds};
                auto const  otherBorder {other->current_style()->Border.Size.calc(o.width())};

                f32 const otherLeft {o.left() - otherBorder};
                f32 const otherRight {o.right() + otherBorder};
                f32 const otherTop {o.top() - otherBorder};
                f32 const otherBottom {o.bottom() + otherBorder};

                bool const overlapY {bottom >= otherTop && top <= otherBottom};
                bool const overlapX {right >= otherLeft && left <= otherRight};

                if (!hasSnappedX && overlapY) {
                    f32 const distL {std::abs(left - otherRight)};
                    f32 const distR {std::abs(right - otherLeft)};

                    if (distL <= _distance && distL <= distR) {
                        b.Position.X = otherRight;
                        hasSnappedX  = true;
                    } else if (distR <= _distance) {
                        b.Position.X = otherLeft - b.width();
                        hasSnappedX  = true;
                    }
                }

                if (!hasSnappedY && overlapX) {
                    f32 const distT {std::abs(top - otherBottom)};
                    f32 const distB {std::abs(bottom - otherTop)};

                    if (distT <= _distance && distT <= distB) {
                        b.Position.Y = otherBottom;
                        hasSnappedY  = true;
                    } else if (distB <= _distance) {
                        b.Position.Y = otherTop - b.height();
                        hasSnappedY  = true;
                    }
                }

                if (hasSnappedX && hasSnappedY) { break; }
            }
        }

        widget->Bounds = b;
    }
}

} // namespace ui
