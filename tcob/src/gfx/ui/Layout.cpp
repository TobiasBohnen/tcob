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
#include "tcob/gfx/ui/Form.hpp"
#include "tcob/gfx/ui/UI.hpp"
#include "tcob/gfx/ui/widgets/Widget.hpp"
#include "tcob/gfx/ui/widgets/WidgetContainer.hpp"

namespace tcob::gfx::ui {

layout::layout(parent parent)
    : _parent {parent}
{
}

void layout::update()
{
    if (!_isDirty) { return; }
    std::stable_sort(_widgets.begin(), _widgets.end(), [](auto const& a, auto const& b) { return a->ZOrder() > b->ZOrder(); });

    std::visit(
        overloaded {
            [this](widget_container* parent) {
                do_layout(parent->content_bounds().Size);
            },
            [this](form* parent) {
                do_layout(parent->Bounds().Size);
            }},
        _parent);
    _isDirty = false;
}

void layout::mark_dirty()
{
    _isDirty = true;
}

void layout::remove_widget(widget* widget)
{
    for (usize i {0}; i < _widgets.size(); ++i) {
        if (_widgets[i].get() == widget) {
            _widgets.erase(_widgets.begin() + i);
            break;
        }
    }
    _isDirty = true;
}

void layout::clear()
{
    _widgets.clear();
    mark_dirty();
}

auto layout::widgets() const -> std::vector<std::shared_ptr<widget>> const&
{
    return _widgets;
}

auto layout::widgets() -> std::vector<std::shared_ptr<widget>>&
{
    return _widgets;
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
            [&retValue, &name](form* parent) {
                retValue.Form   = parent;
                retValue.Parent = nullptr;
                parent->force_redraw(name + ": created");
            }},
        _parent);

    return retValue;
}

////////////////////////////////////////////////////////////

void fixed_layout::do_layout(size_f /* size */)
{
}

////////////////////////////////////////////////////////////

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

void dock_layout::do_layout(size_f size)
{
    auto const& w {widgets()};

    rect_f layoutRect {point_f::Zero, size};
    for (auto const& widget : w) {
        if (layoutRect.height() <= 0 || layoutRect.width() <= 0) {
            widget->Bounds = rect_f::Zero;
            continue;
        }

        f32 const width {std::min(layoutRect.width(), widget->Flex->Width.calc(size.Width))};     // TODO: replace with preferred size
        f32 const height {std::min(layoutRect.height(), widget->Flex->Height.calc(size.Height))}; // TODO: replace with preferred size

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

    for (i32 i {0}; i < std::ssize(w) && i < _box.Width * _box.Height; ++i) {
        auto const& widget {w[i]};
        widget->Bounds = {i % _box.Width * horiSize, i / _box.Width * vertSize,
                          widget->Flex->Width.calc(horiSize),
                          widget->Flex->Height.calc(vertSize)};
    }
}

////////////////////////////////////////////////////////////

void horizontal_layout::do_layout(size_f size)
{
    auto const& w {widgets()};

    f32 const horiSize {size.Width / w.size()};
    f32 const vertSize {size.Height};

    for (i32 i {0}; i < std::ssize(w); ++i) {
        auto const& widget {w[i]};
        widget->Bounds = {i * horiSize, 0,
                          widget->Flex->Width.calc(horiSize),
                          widget->Flex->Height.calc(vertSize)};
    }
}

////////////////////////////////////////////////////////////

void vertical_layout::do_layout(size_f size)
{
    auto& w {widgets()};

    f32 const horiSize {size.Width};
    f32 const vertSize {size.Height / w.size()};

    for (i32 i {0}; i < std::ssize(w); ++i) {
        auto const& widget {w[i]};
        widget->Bounds = {0, i * vertSize,
                          widget->Flex->Width.calc(horiSize),
                          widget->Flex->Height.calc(vertSize)};
    }
}

////////////////////////////////////////////////////////////

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

} // namespace ui
