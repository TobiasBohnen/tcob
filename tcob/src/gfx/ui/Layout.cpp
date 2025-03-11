// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/ui/Layout.hpp"

#include <algorithm>
#include <iterator>
#include <limits>
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

void layout::apply()
{
    ensure_zorder();

    std::visit(
        overloaded {
            [this](widget_container* parent) {
                do_layout(parent->content_bounds().Size);
            },
            [this](form_base* parent) {
                do_layout(parent->Bounds().Size);
            }},
        _parent);
}

void layout::remove_widget(widget* widget)
{
    for (usize i {0}; i < _widgets.size(); ++i) {
        if (_widgets[i].get() == widget) {
            _widgets.erase(_widgets.begin() + i);
            notify_parent();
            return;
        }
    }
}

void layout::clear()
{
    _widgets.clear();
    notify_parent();
}

auto layout::widgets() const -> std::vector<std::shared_ptr<widget>> const&
{
    return _widgets;
}

auto layout::widgets() -> std::vector<std::shared_ptr<widget>>&
{
    return _widgets;
}

void layout::bring_to_front(widget* widget)
{
    if (!widget) { return; }

    isize maxZ {std::numeric_limits<isize>::min()};
    for (auto const& w : _widgets) {
        maxZ = std::max(maxZ, w->ZOrder());
    }
    widget->ZOrder = maxZ + 1;

    ensure_zorder();
}

void layout::send_to_back(widget* widget)
{
    if (!widget) { return; }

    isize minZ {std::numeric_limits<isize>::max()};
    for (auto const& w : _widgets) {
        minZ = std::min(minZ, w->ZOrder());
    }
    widget->ZOrder = minZ - 1;

    ensure_zorder();
}

auto layout::is_resize_allowed() const -> bool
{
    return false;
}

auto layout::is_move_allowed() const -> bool
{
    return false;
}

void layout::notify_parent()
{
    std::visit(
        overloaded {
            [](widget_container* parent) {
                parent->force_redraw("layout updated");
            },
            [](form_base* parent) {
                parent->force_redraw("layout updated");
            }},
        _parent);
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
            [&retValue, &name](widget_container* parent) {
                retValue.Form   = parent->parent_form();
                retValue.Parent = parent;
                parent->force_redraw(name + ": created");
            },
            [&retValue, &name](form_base* parent) {
                retValue.Form   = parent;
                retValue.Parent = nullptr;
                parent->force_redraw(name + ": created");
            }},
        _parent);

    return retValue;
}

void layout::ensure_zorder()
{
    std::ranges::stable_sort(_widgets, [](auto const& a, auto const& b) { return a->ZOrder() > b->ZOrder(); });
}

////////////////////////////////////////////////////////////

static_layout::static_layout(parent parent)
    : layout {parent}
{
}

auto static_layout::is_resize_allowed() const -> bool
{
    return true;
}

auto static_layout::is_move_allowed() const -> bool
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

auto flex_size_layout::is_move_allowed() const -> bool
{
    return true;
}

void flex_size_layout::do_layout(size_f size)
{
    auto const& w {widgets()};

    for (auto const& widget : w) {
        auto bounds {widget->Bounds()};
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

grid_layout::grid_layout(parent parent, size_i initSize)
    : layout {parent}
    , _grid {initSize}
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
        bounds.Size.Width *= horiSize;
        bounds.Size.Width = widget->Flex->Width.calc(bounds.Size.Width);
        bounds.Position.Y *= vertSize;
        bounds.Size.Height *= vertSize;
        bounds.Size.Height = widget->Flex->Height.calc(bounds.Size.Height);

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
            widget->Bounds = {i % _box.Width * horiSize, i / _box.Width * vertSize,
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
        case gfx::vertical_alignment::Top: widget->Bounds = {x, 0, width, height}; break;
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
        case gfx::horizontal_alignment::Left: widget->Bounds = {0, y, width, height}; break;
        case gfx::horizontal_alignment::Right: widget->Bounds = {horiSize - width, y, width, height}; break;
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
    , _columns {columns} // _columns is a member variable
{
}

void masonry_layout::do_layout(size_f size)
{
    auto const&      w {widgets()};
    f32 const        colWidth {size.Width / _columns};
    std::vector<f32> colHeights(_columns, 0.0f);

    for (auto const& widget : w) {
        f32 const widgetWidth {widget->Flex->Width.calc(colWidth)};
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
        f32 const x {colIndex * colWidth};
        f32 const y {colHeights[colIndex]};
        widget->Bounds = {x, y, widgetWidth, widgetHeight};
        colHeights[colIndex] += widgetHeight;
    }
}

} // namespace ui
