// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "Form.hpp"

#include <algorithm>
#include <memory>
#include <span>
#include <unordered_set>
#include <vector>

#include "tcob/core/Point.hpp"
#include "tcob/core/Rect.hpp"
#include "tcob/core/Size.hpp"
#include "tcob/gfx/ui/UI.hpp"
#include "tcob/gfx/ui/widgets/Tooltip.hpp"
#include "tcob/gfx/ui/widgets/Widget.hpp"

namespace tcob::ui {

////////////////////////////////////////////////////////////

template <DerivedFrom<layout> Layout>
inline form<Layout>::form(form_init const& init, auto&&... layoutArgs)
    : form_base {init.Name, rect_f {init.Bounds}}
    , _layout {this, layoutArgs...}
{
    _layout.Changed.connect([&] { queue_redraw(); });
}

template <DerivedFrom<layout> Layout>
template <DerivedFrom<widget_container> T>
inline auto form<Layout>::create_container(auto&&... args) -> T&
{
    return _layout.template create_widget<T>(args...);
}

template <DerivedFrom<layout> Layout>
inline auto form<Layout>::containers() const -> std::span<std::unique_ptr<widget> const>
{
    return _layout.widgets();
}

template <DerivedFrom<layout> Layout>
inline void form<Layout>::remove_container(widget const& widget)
{
    _layout.remove(widget);
}

template <DerivedFrom<layout> Layout>
inline void form<Layout>::clear_containers()
{
    _layout.clear();
}

template <DerivedFrom<layout> Layout>
inline void form<Layout>::apply_layout()
{
    _layout.apply(Bounds->Size);
}

template <DerivedFrom<layout> Layout>
inline auto form<Layout>::get_layout() -> layout*
{
    return &_layout;
}

template <DerivedFrom<layout> Layout>
inline auto form<Layout>::get_layout() const -> layout const*
{
    return &_layout;
}

template <DerivedFrom<layout> Layout>
inline auto form<Layout>::allows_move() const -> bool
{
    return _layout.allows_move();
}

template <DerivedFrom<layout> Layout>
inline auto form<Layout>::allows_resize() const -> bool
{
    return _layout.allows_resize();
}

////////////////////////////////////////////////////////////

template <DerivedFrom<tooltip> T>
inline auto form_base::create_tooltip(string const& name) -> std::shared_ptr<T>
{
    widget::init const wi {
        .Form   = this,
        .Parent = nullptr,
        .Name   = name,
    };

    auto retValue {std::make_shared<T>(wi)};
    _tooltips.push_back(retValue);
    return retValue;
}

template <DerivedFrom<toast> T>
inline auto form_base::queue_toast(string const& name, corner corner) -> T&
{
    widget::init const wi {
        .Form   = this,
        .Parent = nullptr,
        .Name   = name,
    };
    auto ptr {std::make_unique<T>(wi)};
    ptr->_corner = corner;
    auto*      retValue {ptr.get()};
    auto const fb {bounds()};
    auto const size {size_f {fb.width() / 5.f, fb.height() / 5.f}};

    i32 const maxRows {static_cast<i32>(fb.height() / size.Height)};

    std::unordered_set<point_i> occupied;
    for (auto const& t : _toasts) {
        if (t->_corner == corner) {
            occupied.insert(t->_slot);
        }
    }

    point_i slot {0, 0};
    while (occupied.contains(slot)) {
        ++slot.Y;
        if (slot.Y >= maxRows) {
            slot.Y = 0;
            ++slot.X;
        }
    }

    point_f pos {};

    switch (corner) {
    case corner::TopLeft:
        pos.X = fb.left() + (static_cast<f32>(slot.X) * size.Width);
        pos.Y = fb.top() + (static_cast<f32>(slot.Y) * size.Height);
        break;
    case corner::TopRight:
        pos.X = fb.right() - size.Width - (static_cast<f32>(slot.X) * size.Width);
        pos.Y = fb.top() + (static_cast<f32>(slot.Y) * size.Height);
        break;
    case corner::BottomLeft:
        pos.X = fb.left() + (static_cast<f32>(slot.X) * size.Width);
        pos.Y = fb.bottom() - size.Height - (static_cast<f32>(slot.Y) * size.Height);
        break;
    case corner::BottomRight:
        pos.X = fb.right() - size.Width - (static_cast<f32>(slot.X) * size.Width);
        pos.Y = fb.bottom() - size.Height - (static_cast<f32>(slot.Y) * size.Height);
        break;
    }

    retValue->_slot  = slot;
    retValue->Bounds = {pos, size};
    _toasts.push_back(std::move(ptr));
    return *retValue;
}

template <DerivedFrom<modal_dialog> T>
inline auto form_base::create_modal_dialog(string const& name) -> std::shared_ptr<T>
{
    widget::init const wi {
        .Form   = this,
        .Parent = nullptr,
        .Name   = name,
    };

    return std::make_shared<T>(wi);
}

template <SubmitTarget Target>
inline void form_base::submit(Target& target)
{
    auto widgets {all_widgets()};
    for (auto* widget : widgets) {
        auto const props {widget->attributes()};
        auto const name {widget->name()};
        if (!props.empty() && !name.empty()) {
            target[name] = props;
        }
    }
}
}
