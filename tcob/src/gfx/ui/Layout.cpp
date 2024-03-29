// Copyright (c) 2023 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/ui/Layout.hpp"

#include "tcob/core/Common.hpp"
#include "tcob/gfx/ui/Form.hpp"
#include "tcob/gfx/ui/widgets/WidgetContainer.hpp"

namespace tcob::gfx::ui {

layout::layout(parent parent)
    : _parent {parent}
{
}

void layout::update()
{
    if (_isDirty) {
        std::visit(
            tcob::detail::overloaded {
                [&](widget_container* parent) {
                    do_layout(parent->get_content_bounds().get_size());
                },
                [&](form* parent) {
                    do_layout(parent->Bounds().get_size());
                }},
            _parent);

        _isDirty = false;
    }
}

void layout::mark_dirty()
{
    _isDirty = true;
}

void layout::clear()
{
    _widgets.clear();
    mark_dirty();
}

auto layout::get_widgets() -> std::vector<std::shared_ptr<widget>>&
{
    return _widgets;
}

auto layout::create_init(string const& name) const -> widget::init
{
    widget::init retValue {};
    retValue.Name = name;

    std::visit(
        tcob::detail::overloaded {
            [&](widget_container* parent) {
                retValue.Form   = parent->get_form();
                retValue.Parent = parent;
                parent->force_redraw(name + ": created");
            },
            [&](form* parent) {
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
    auto& widgets {get_widgets()};

    for (auto const& widget : widgets) {
        (*widget->Bounds).Width  = widget->Flex->Width.calc(size.Width);
        (*widget->Bounds).Height = widget->Flex->Height.calc(size.Height);
    }
}

////////////////////////////////////////////////////////////

void dock_layout::do_layout(size_f size)
{
    auto& widgets {get_widgets()};

    rect_f layoutRect {point_f::Zero, size};
    for (auto const& widget : widgets) {
        if (layoutRect.Height <= 0 || layoutRect.Width <= 0) {
            break;
        }

        f32 const width {std::min(layoutRect.Width, widget->Flex->Width.calc(size.Width))};     // TODO: replace with preferred size
        f32 const height {std::min(layoutRect.Height, widget->Flex->Height.calc(size.Height))}; // TODO: replace with preferred size

        rect_f cbounds {layoutRect};

        switch (_widgetDock[widget.get()]) {
        case dock_style::Left:
            cbounds.Width = width;

            layoutRect.X += width;
            layoutRect.Width -= width;
            break;
        case dock_style::Right:
            cbounds.X     = cbounds.Width - width;
            cbounds.Width = width;

            layoutRect.Width -= width;
            break;
        case dock_style::Top:
            cbounds.Height = height;

            layoutRect.Y += height;
            layoutRect.Height -= height;
            break;
        case dock_style::Bottom:
            cbounds.Y      = cbounds.Height - height;
            cbounds.Height = height;

            layoutRect.Height -= height;
            break;
        case dock_style::Fill:
            cbounds.Width  = width;
            cbounds.Height = height;

            layoutRect.X += width;
            layoutRect.Width -= width;
            layoutRect.Y += height;
            layoutRect.Height -= height;
            break;
        }

        widget->Bounds = cbounds;
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
    auto& widgets {get_widgets()};

    f32 const horiSize {size.Width / _grid.Width};
    f32 const vertSize {size.Height / _grid.Height};

    for (auto const& widget : widgets) {
        rect_f bounds {_widgetBounds[widget.get()]};
        bounds.X *= horiSize;
        bounds.Width *= horiSize;
        bounds.Width = widget->Flex->Width.calc(bounds.Width);
        bounds.Y *= vertSize;
        bounds.Height *= vertSize;
        bounds.Height = widget->Flex->Height.calc(bounds.Height);

        widget->Bounds = bounds;
    }
}

////////////////////////////////////////////////////////////

void hbox_layout::do_layout(size_f size)
{
    auto& widgets {get_widgets()};

    f32 const horiSize {size.Width / widgets.size()};
    f32 const vertSize {size.Height};

    for (i32 i {0}; i < std::ssize(widgets); ++i) {
        auto& widget {widgets[i]};
        widget->Bounds =
            {i * horiSize, 0,
             widget->Flex->Width.calc(horiSize),
             widget->Flex->Height.calc(vertSize)};
    }
}

////////////////////////////////////////////////////////////

void vbox_layout::do_layout(size_f size)
{
    auto& widgets {get_widgets()};

    f32 const horiSize {size.Width};
    f32 const vertSize {size.Height / widgets.size()};

    for (i32 i {0}; i < std::ssize(widgets); ++i) {
        auto& widget {widgets[i]};
        widget->Bounds =
            {0, i * vertSize,
             widget->Flex->Width.calc(horiSize),
             widget->Flex->Height.calc(vertSize)};
    }
}

} // namespace ui
