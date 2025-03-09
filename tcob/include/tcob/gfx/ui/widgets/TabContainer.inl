// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "TabContainer.hpp"

#include <memory>

#include "tcob/gfx/ui/UI.hpp"
#include "tcob/gfx/ui/widgets/Widget.hpp"
#include "tcob/gfx/ui/widgets/WidgetContainer.hpp"

namespace tcob::ui {

template <std::derived_from<widget_container> T>
inline auto tab_container::create_tab(utf8_string const& name) -> std::shared_ptr<T>
{
    return create_tab<T>(name, {.Text = name, .Icon = {}, .UserData = {}});
}

template <std::derived_from<widget_container> T>
inline auto tab_container::create_tab(utf8_string const& name, list_item const& label) -> std::shared_ptr<T>
{
    force_redraw(this->name() + ": tab created");

    widget::init wi {};
    wi.Form   = parent_form();
    wi.Parent = this;
    wi.Name   = name;

    auto retValue {std::make_shared<T>(wi)};
    _tabs.push_back(retValue);

    _tabLabels.push_back(label);
    if (ActiveTabIndex == -1) {
        ActiveTabIndex = 0;
    }
    return retValue;
}
}
