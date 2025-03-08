// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "Accordion.hpp"

#include <memory>

#include "tcob/gfx/ui/UI.hpp"
#include "tcob/gfx/ui/widgets/WidgetContainer.hpp"

namespace tcob::gfx::ui {

template <std::derived_from<widget_container> T>
inline auto accordion::create_section(utf8_string const& name) -> std::shared_ptr<T>
{
    return create_section<T>(name, {.Text = name, .Icon = {}, .UserData = {}});
}

template <std::derived_from<widget_container> T>
inline auto accordion::create_section(utf8_string const& name, list_item const& label) -> std::shared_ptr<T>
{
    force_redraw(this->name() + ": section created");

    widget::init wi {};
    wi.Form   = parent_form();
    wi.Parent = this;
    wi.Name   = name;

    auto retValue {std::make_shared<T>(wi)};
    _sections.push_back(retValue);
    _sectionLabels.push_back(label);

    return retValue;
}
}
