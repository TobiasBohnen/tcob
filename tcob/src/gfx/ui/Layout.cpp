// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/ui/Layout.hpp"

#include <algorithm>
#include <cstdlib>
#include <iterator>
#include <memory>
#include <numeric>
#include <span>
#include <utility>
#include <vector>

#include "tcob/core/AngleUnits.hpp"
#include "tcob/core/Common.hpp"
#include "tcob/core/Point.hpp"
#include "tcob/core/Rect.hpp"
#include "tcob/core/Size.hpp"
#include "tcob/core/StringUtils.hpp"
#include "tcob/gfx/Gfx.hpp"
#include "tcob/gfx/ui/Form.hpp"
#include "tcob/gfx/ui/Length.hpp"
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

void layout::remove(widget const& target)
{
    for (usize i {0}; i < _widgets.size(); ++i) {
        if (_widgets[i].get() == &target) {
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

auto layout::widgets() const -> std::span<std::unique_ptr<widget> const>
{
    return _widgets;
}

void layout::bring_to_front(widget& target)
{
    if (_widgets.empty()) { return; }
    target.ZOrder = _widgets.size() + 1;
    normalize_zorder();
}

void layout::send_to_back(widget& target)
{
    if (_widgets.empty()) { return; }
    target.ZOrder = 0;
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

    overloaded_visit(
        _parent,
        [this, &retValue](widget_container* parent) {
            retValue.Form   = &parent->form();
            retValue.Parent = parent;
            Changed();
        },
        [this, &retValue](form_base* parent) {
            retValue.Form   = parent;
            retValue.Parent = nullptr;
            Changed();
        });

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

auto_size_layout::auto_size_layout(parent parent)
    : layout {parent}
{
}

auto auto_size_layout::allows_move() const -> bool
{
    return true;
}

void auto_size_layout::do_layout(size_f size)
{
    auto const& w {widgets()};

    for (auto const& widget : w) {
        auto bounds {*widget->Bounds};
        bounds.Size.Width  = widget->RelativeSize->Width * size.Width;
        bounds.Size.Height = widget->RelativeSize->Height * size.Height;
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

        f32 const width {std::min(layoutRect.width(), widget->RelativeSize->Width * size.Width)};
        f32 const height {std::min(layoutRect.height(), widget->RelativeSize->Height * size.Height)};

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
        bounds.Size.Width = widget->RelativeSize->Width * (bounds.Size.Width * horiSize);
        bounds.Position.Y *= vertSize;
        bounds.Size.Height = widget->RelativeSize->Height * (bounds.Size.Height * vertSize);

        widget->Bounds = bounds;
    }
}

////////////////////////////////////////////////////////////

static auto expand_to_weights(std::span<i32 const> sizes) -> weights_t
{
    weights_t weights;
    weights.reserve(sizes.size());
    for (i32 n : sizes) { weights.emplace_back(n, 1.f); }
    return weights;
}

horizontal_layout::horizontal_layout(parent parent, weights_t columnWeights, gfx::vertical_alignment alignment)
    : layout {parent}
    , _weights {std::move(columnWeights)}
    , _alignment {alignment}
{
}

horizontal_layout::horizontal_layout(parent parent, std::span<i32 const> columns, gfx::vertical_alignment alignment)
    : horizontal_layout {parent, expand_to_weights(columns), alignment}
{
}

horizontal_layout::horizontal_layout(parent parent, gfx::vertical_alignment alignment)
    : layout {parent}
    , _alignment {alignment}
    , _autoWeights {true}
{
}

void horizontal_layout::do_layout(size_f size)
{
    auto const& w {widgets()};
    if (w.empty()) { return; }
    if (_autoWeights) {
        _weights = {{std::vector<f32>(w.size(), 1.f)}};
    }

    f32 const rowHeight {size.Height / static_cast<f32>(_weights.size())};

    usize idx {0};
    for (i32 row {0}; row < std::ssize(_weights) && idx < w.size(); ++row) {
        auto const& weights {_weights[row]};
        f32 const   totalWeight {std::accumulate(weights.begin(), weights.end(), 0.f)};
        f32         x {0.f};

        for (i32 col {0}; col < std::ssize(weights) && idx < w.size(); ++col, ++idx) {
            f32 const cellWidth {size.Width * (weights[col] / totalWeight)};
            f32 const widgetWidth {w[idx]->RelativeSize->Width * cellWidth};
            f32 const widgetHeight {w[idx]->RelativeSize->Height * rowHeight};

            f32 y {row * rowHeight};
            switch (_alignment) {
            case gfx::vertical_alignment::Top:    break;
            case gfx::vertical_alignment::Bottom: y += rowHeight - widgetHeight; break;
            case gfx::vertical_alignment::Middle: y += (rowHeight - widgetHeight) / 2.f; break;
            }

            w[idx]->Bounds = {x, y, widgetWidth, widgetHeight};
            x += cellWidth;
        }
    }

    for (; idx < w.size(); ++idx) { w[idx]->Bounds = rect_f::Zero; }
}

////////////////////////////////////////////////////////////

vertical_layout::vertical_layout(parent parent, weights_t rowHeights, gfx::horizontal_alignment alignment)
    : layout {parent}
    , _weights {std::move(rowHeights)}
    , _alignment {alignment}
{
}

vertical_layout::vertical_layout(parent parent, std::span<i32 const> rows, gfx::horizontal_alignment alignment)
    : vertical_layout {parent, expand_to_weights(rows), alignment}
{
}

vertical_layout::vertical_layout(parent parent, gfx::horizontal_alignment alignment)
    : layout {parent}
    , _alignment {alignment}
    , _autoWeights {true}
{
}

void vertical_layout::do_layout(size_f size)
{
    auto const& w {widgets()};
    if (w.empty()) { return; }
    if (_autoWeights) {
        _weights = {{std::vector<f32>(w.size(), 1.f)}};
    }

    f32 const colWidth {size.Width / static_cast<f32>(_weights.size())};

    usize idx {0};
    for (i32 col {0}; col < std::ssize(_weights) && idx < w.size(); ++col) {
        auto const& weights {_weights[col]};
        f32 const   totalWeight {std::accumulate(weights.begin(), weights.end(), 0.f)};
        f32         y {0.f};

        for (i32 row {0}; row < std::ssize(weights) && idx < w.size(); ++row, ++idx) {
            f32 const cellHeight {size.Height * (weights[row] / totalWeight)};
            f32 const widgetWidth {w[idx]->RelativeSize->Width * colWidth};
            f32 const widgetHeight {w[idx]->RelativeSize->Height * cellHeight};

            f32 x {col * colWidth};
            switch (_alignment) {
            case gfx::horizontal_alignment::Left:     break;
            case gfx::horizontal_alignment::Right:    x += colWidth - widgetWidth; break;
            case gfx::horizontal_alignment::Centered: x += (colWidth - widgetWidth) / 2.f; break;
            }

            w[idx]->Bounds = {x, y, widgetWidth, widgetHeight};
            y += cellHeight;
        }
    }

    for (; idx < w.size(); ++idx) { w[idx]->Bounds = rect_f::Zero; }
}

////////////////////////////////////////////////////////////

tile_layout::tile_layout(parent parent, size_i boxSize)
    : layout {parent}
    , _box {boxSize}
{
}

void tile_layout::do_layout(size_f size)
{
    auto const& w {widgets()};
    f32 const   horiSize {size.Width / _box.Width};
    f32 const   vertSize {size.Height / _box.Height};
    for (i32 i {0}; i < std::ssize(w); ++i) {
        auto const& widget {w[i]};
        if (i < _box.Width * _box.Height) {
            f32 const cellX {static_cast<f32>(i % _box.Width) * horiSize};
            f32 const cellY {static_cast<f32>(i / _box.Width) * vertSize};
            f32 const widgetWidth {widget->RelativeSize->Width * horiSize};
            f32 const widgetHeight {widget->RelativeSize->Height * vertSize};

            widget->Bounds = {cellX + ((horiSize - widgetWidth) / 2.0f),
                              cellY + ((vertSize - widgetHeight) / 2.0f),
                              widgetWidth,
                              widgetHeight};
        } else {
            widget->Bounds = rect_f::Zero;
        }
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
        f32 const widgetWidth {widget->RelativeSize->Width * availableWidth};
        f32 const widgetHeight {widget->RelativeSize->Height * size.Height};

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

masonry_layout::masonry_layout(parent parent, i32 tracks, direction dir)
    : layout {parent}
    , _tracks {tracks}
    , _dir {dir}
{
}

void masonry_layout::do_layout(size_f size)
{
    auto const& w {widgets()};

    bool const isVertical {_dir == direction::Down || _dir == direction::Up};

    f32 const        trackSize {isVertical ? size.Width / static_cast<f32>(_tracks) : size.Height / static_cast<f32>(_tracks)};
    std::vector<f32> trackFill(_tracks, 0.f);

    for (auto const& widget : w) {
        f32 const widgetWidth {widget->RelativeSize->Width * (isVertical ? trackSize : size.Width)};
        f32 const widgetHeight {widget->RelativeSize->Height * (isVertical ? size.Height : trackSize)};
        f32 const widgetMain {isVertical ? widgetHeight : widgetWidth};
        f32 const mainSize {isVertical ? size.Height : size.Width};

        // find shortest track that fits
        i32 trackIndex {-1};
        for (i32 i {0}; i < _tracks; ++i) {
            if (trackFill[i] + widgetMain <= mainSize && (trackIndex == -1 || trackFill[i] < trackFill[trackIndex])) {
                trackIndex = i;
            }
        }

        // If no track can fit the widget, skip it
        if (trackIndex == -1) {
            widget->Bounds = rect_f::Zero;
            continue;
        }

        // Place the widget in the selected track
        f32 const cross {trackIndex * trackSize};
        f32 const main {trackFill[trackIndex]};

        switch (_dir) {
        case direction::Down:  widget->Bounds = {cross, main, widgetWidth, widgetHeight}; break;
        case direction::Up:    widget->Bounds = {cross, mainSize - main - widgetHeight, widgetWidth, widgetHeight}; break;
        case direction::Right: widget->Bounds = {main, cross, widgetWidth, widgetHeight}; break;
        case direction::Left:  widget->Bounds = {mainSize - main - widgetWidth, cross, widgetWidth, widgetHeight}; break;
        case direction::None:  widget->Bounds = rect_f::Zero; break;
        }

        trackFill[trackIndex] += widgetMain;
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
        f32 const widgetWidth {widget->RelativeSize->Width * horiSize};
        f32 const widgetHeight {widget->RelativeSize->Height * vertSize};

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
            rect_f const bounds {point_f::Zero, {widget->RelativeSize->Width * (horiSize), widget->RelativeSize->Height * (vertSize)}};
            if (widget->Bounds != bounds) {
                widget->Bounds = bounds;
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
        f32 const   widgetWidth {widget->RelativeSize->Width * horiSize};
        f32 const   widgetHeight {widget->RelativeSize->Height * vertSize};

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

}
