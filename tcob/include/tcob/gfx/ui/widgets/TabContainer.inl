// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "TabContainer.hpp"

#include <memory>

#include "tcob/gfx/ui/UI.hpp"
#include "tcob/gfx/ui/component/Item.hpp"
#include "tcob/gfx/ui/widgets/Widget.hpp"
#include "tcob/gfx/ui/widgets/WidgetContainer.hpp"

namespace tcob::ui {

template <std::derived_from<widget_container> T>
inline auto tab_container::create_tab(utf8_string const& name) -> T&
{
    return create_tab<T>(name, {.Text = name, .Icon = {}, .UserData = {}});
}

template <std::derived_from<widget_container> T>
inline auto tab_container::create_tab(utf8_string const& name, item const& label) -> T&
{
    queue_redraw();

    widget::init const wi {
        .Form   = &form(),
        .Parent = this,
        .Name   = name,
    };

    auto  ptr {std::make_unique<T>(wi)};
    auto& retValue {*ptr};
    _tabs.push_back(std::move(ptr));
    _tabLabels.push_back(label);
    if (ActiveTabIndex == -1) { ActiveTabIndex = 0; }

    return retValue;
}
}
