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
inline auto form_base::queue_toast(string const& name, corner corner, size_i size) -> T&
{
    constexpr size_i TOAST_GRID_SIZE {10, 10};
    constexpr usize  MAX_TOASTS {10};

    widget::init const wi {
        .Form   = this,
        .Parent = nullptr,
        .Name   = name,
    };
    auto  ptr {std::make_unique<T>(wi, corner)};
    auto* retValue {ptr.get()};

    if (_toasts.size() >= MAX_TOASTS) { _toasts.erase(_toasts.begin()); }

    auto const fb {bounds()};
    auto const slotSize {size_f {fb.width() / static_cast<f32>(TOAST_GRID_SIZE.Width), fb.height() / static_cast<f32>(TOAST_GRID_SIZE.Height)}};
    auto const toastSize {size_f {slotSize.Width * size.Width, slotSize.Height * size.Height}};

    std::unordered_set<point_i> occupied;
    for (auto const& t : _toasts) {
        occupied.insert(t->_slots.begin(), t->_slots.end());
    }

    auto const fits {[&](point_i slot) {
        for (i32 col {0}; col < size.Width; ++col) {
            for (i32 row {0}; row < size.Height; ++row) {
                if (occupied.contains({slot.X + col, slot.Y + row})) { return false; }
            }
        }
        return true;
    }};

    point_i slot {};
    switch (corner) {
    case corner::TopLeft:
        slot = {0, 0};
        while (!fits(slot)) {
            ++slot.Y;
            if (slot.Y + size.Height > TOAST_GRID_SIZE.Height) {
                slot.Y = 0;
                ++slot.X;
            }
        }
        break;
    case corner::TopRight:
        slot = {TOAST_GRID_SIZE.Width - size.Width, 0};
        while (!fits(slot)) {
            ++slot.Y;
            if (slot.Y + size.Height > TOAST_GRID_SIZE.Height) {
                slot.Y = 0;
                --slot.X;
            }
        }
        break;
    case corner::BottomLeft:
        slot = {0, TOAST_GRID_SIZE.Height - size.Height};
        while (!fits(slot)) {
            --slot.Y;
            if (slot.Y < 0) {
                slot.Y = TOAST_GRID_SIZE.Height - size.Height;
                ++slot.X;
            }
        }
        break;
    case corner::BottomRight:
        slot = {TOAST_GRID_SIZE.Width - size.Width, TOAST_GRID_SIZE.Height - size.Height};
        while (!fits(slot)) {
            --slot.Y;
            if (slot.Y < 0) {
                slot.Y = TOAST_GRID_SIZE.Height - size.Height;
                --slot.X;
            }
        }
        break;
    }

    for (i32 col {0}; col < size.Width; ++col) {
        for (i32 row {0}; row < size.Height; ++row) {
            retValue->_slots.insert({slot.X + col, slot.Y + row});
        }
    }

    point_f const pos {
        fb.left() + (static_cast<f32>(slot.X) * slotSize.Width),
        fb.top() + (static_cast<f32>(slot.Y) * slotSize.Height)};

    retValue->Bounds = {pos, toastSize};
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
