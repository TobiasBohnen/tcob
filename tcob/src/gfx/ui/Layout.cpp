// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/ui/Layout.hpp"

#include <algorithm>
#include <iterator>
#include <memory>
#include <variant>
#include <vector>

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

auto layout::widgets() const -> std::vector<std::shared_ptr<widget>> const&
{
    return _widgets;
}

auto layout::widgets() -> std::vector<std::shared_ptr<widget>>&
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

static_layout::static_layout(parent parent)
    : layout {parent}
{
}

auto static_layout::allows_move() const -> bool
{
    return true;
}

auto static_layout::allows_resize() const -> bool
{
    return true;
}

void static_layout::do_layout(size_f /* size */)
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

    f32 const horiSize {size.Width / w.size()};
    f32 const vertSize {size.Height};

    for (i32 i {0}; i < std::ssize(w); ++i) {
        auto const& widget {w[i]};
        f32 const   width {widget->Flex->Width.calc(horiSize)};
        f32 const   height {widget->Flex->Height.calc(vertSize)};
        f32 const   x {i * horiSize};

        switch (_alignment) {
        case gfx::vertical_alignment::Top:    widget->Bounds = {x, 0, width, height}; break;
        case gfx::vertical_alignment::Bottom: widget->Bounds = {x, vertSize - height, width, height}; break;
        case gfx::vertical_alignment::Middle: widget->Bounds = {x, (vertSize - height) / 2, width, height}; break;
        }
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
    auto& w {widgets()};

    f32 const horiSize {size.Width};
    f32 const vertSize {size.Height / w.size()};

    for (i32 i {0}; i < std::ssize(w); ++i) {
        auto const& widget {w[i]};
        f32 const   width {widget->Flex->Width.calc(horiSize)};
        f32 const   height {widget->Flex->Height.calc(vertSize)};
        f32 const   y {i * vertSize};

        switch (_alignment) {
        case gfx::horizontal_alignment::Left:     widget->Bounds = {0, y, width, height}; break;
        case gfx::horizontal_alignment::Right:    widget->Bounds = {horiSize - width, y, width, height}; break;
        case gfx::horizontal_alignment::Centered: widget->Bounds = {(horiSize - width) / 2, y, width, height}; break;
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
    auto& w {widgets()};

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

    f32 y {0.f};
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
                widget->form().refresh_hover(widget.get());
            }
        } else {
            widget->Bounds = rect_f::Zero;
        }
    }
}

} // namespace ui
