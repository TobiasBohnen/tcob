// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "Layout.hpp"

#include <algorithm>
#include <memory>

#include "tcob/core/Point.hpp"
#include "tcob/core/Rect.hpp"
#include "tcob/core/Size.hpp"
#include "tcob/gfx/ui/UI.hpp"
#include "tcob/gfx/ui/widgets/Widget.hpp"

namespace tcob::ui {

////////////////////////////////////////////////////////////

namespace detail {
    template <typename Derived>
    template <std::derived_from<widget> T>
    inline auto default_creator<Derived>::create_widget(string const& name) -> T&
    {
        return static_cast<Derived*>(this)->template add_widget<T>(name);
    }

}

////////////////////////////////////////////////////////////

template <std::derived_from<widget> T>
inline auto layout::add_widget(string const& name) -> T&
{
    auto const wi {create_init(name)};

    auto  ptr {std::make_unique<T>(wi)};
    auto& retValue {*ptr};
    _widgets.push_back(std::move(ptr));
    normalize_zorder();
    return retValue;
}

////////////////////////////////////////////////////////////

template <std::derived_from<widget> T>
inline auto manual_layout::create_widget(rect_f const& rect, string const& name) -> T&
{
    auto& retValue {add_widget<T>(name)};
    retValue.Bounds = rect;
    return retValue;
}

////////////////////////////////////////////////////////////

template <std::derived_from<widget> T>
inline auto flex_size_layout::create_widget(point_f pos, string const& name) -> T&
{
    auto& retValue {add_widget<T>(name)};
    retValue.Bounds = {pos, size_f::Zero};
    return retValue;
}

////////////////////////////////////////////////////////////

template <std::derived_from<widget> T>
inline auto dock_layout::create_widget(dock_style dock, string const& name) -> T&
{
    auto& retValue {add_widget<T>(name)};
    _widgetDock[&retValue] = dock;
    return retValue;
}

////////////////////////////////////////////////////////////

template <std::derived_from<widget> T>
inline auto grid_layout::create_widget(rect_i const& bounds, string const& name) -> T&
{
    auto& retValue {add_widget<T>(name)};

    _widgetBounds[&retValue] = bounds;
    if (_autoGrow) {
        _grid.Width  = std::max(_grid.Width, bounds.right());
        _grid.Height = std::max(_grid.Height, bounds.bottom());
    }

    return retValue;
}

////////////////////////////////////////////////////////////

template <std::derived_from<widget> T>
inline auto tree_layout::create_widget(i32 level, string const& name) -> T&
{
    auto& retValue {add_widget<T>(name)};

    _levels[&retValue] = level;
    _maxLevel          = std::max(_maxLevel, level);

    return retValue;
}

////////////////////////////////////////////////////////////

template <std::derived_from<widget> T>
inline auto magnetic_snap_layout::create_widget(rect_f const& rect, string const& name) -> T&
{
    auto& retValue {add_widget<T>(name)};
    retValue.Bounds = rect;
    return retValue;
}

////////////////////////////////////////////////////////////

}
