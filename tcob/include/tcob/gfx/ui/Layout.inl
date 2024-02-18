// Copyright (c) 2023 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "Layout.hpp"

namespace tcob::gfx::ui {

template <std::derived_from<widget> T>
inline auto layout::add_widget(string const& name) -> std::shared_ptr<T>
{
    auto const wi {create_widgetinfo(name)};

    auto retValue {std::make_shared<T>(wi)};
    _isDirty = true;
    _widgets.push_back(retValue);

    return retValue;
}

////////////////////////////////////////////////////////////

template <std::derived_from<widget> T>
inline auto fixed_layout::create_widget(rect_f const& rect, string const& name) -> std::shared_ptr<T>
{
    auto retValue {add_widget<T>(name)};
    retValue->Bounds = rect;
    return std::dynamic_pointer_cast<T>(retValue);
}

////////////////////////////////////////////////////////////

template <std::derived_from<widget> T>
inline auto flex_size_layout::create_widget(point_f pos, string const& name) -> std::shared_ptr<T>
{
    auto retValue {add_widget<T>(name)};
    retValue->Bounds = {pos, size_f::Zero};
    return std::dynamic_pointer_cast<T>(retValue);
}

////////////////////////////////////////////////////////////

template <std::derived_from<widget> T>
inline auto dock_layout::create_widget(dock_style dock, string const& name) -> std::shared_ptr<T>
{
    auto retValue {add_widget<T>(name)};
    _widgetDock[retValue.get()] = dock;
    return std::dynamic_pointer_cast<T>(retValue);
}

////////////////////////////////////////////////////////////

template <std::derived_from<widget> T>
inline auto grid_layout::create_widget(rect_i const& bounds, string const& name) -> std::shared_ptr<T>
{
    auto retValue {add_widget<T>(name)};

    _widgetBounds[retValue.get()] = bounds;
    _grid.Width                   = std::max(_grid.Width, bounds.right());
    _grid.Height                  = std::max(_grid.Height, bounds.bottom());

    return std::dynamic_pointer_cast<T>(retValue);
}

////////////////////////////////////////////////////////////

template <std::derived_from<widget> T>
inline auto hbox_layout::create_widget(string const& name) -> std::shared_ptr<T>
{
    auto retValue {add_widget<T>(name)};
    return std::dynamic_pointer_cast<T>(retValue);
}

////////////////////////////////////////////////////////////

template <std::derived_from<widget> T>
inline auto vbox_layout::create_widget(string const& name) -> std::shared_ptr<T>
{
    auto retValue {add_widget<T>(name)};
    return std::dynamic_pointer_cast<T>(retValue);
}

}
