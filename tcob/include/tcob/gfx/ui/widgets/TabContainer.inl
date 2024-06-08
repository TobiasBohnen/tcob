// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "TabContainer.hpp"

namespace tcob::gfx::ui {

template <std::derived_from<widget_container> T>
inline auto tab_container::create_tab(utf8_string const& name) -> std::shared_ptr<T>
{
    return create_tab<T>(name, name);
}

template <std::derived_from<widget_container> T>
inline auto tab_container::create_tab(utf8_string const& name, utf8_string const& label) -> std::shared_ptr<T>
{
    force_redraw(get_name() + ": tab created");

    widget::init wi {};
    wi.Form   = get_form();
    wi.Parent = this;
    wi.Name   = name;

    auto retValue {std::make_shared<T>(wi)};
    _tabs.push_back(retValue);
    // TODO: translation hook
    _tabLabels.push_back(label);
    if (ActiveTabIndex == -1) {
        ActiveTabIndex = 0;
    }
    return retValue;
}
}
