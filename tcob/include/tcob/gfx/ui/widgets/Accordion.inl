// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "Accordion.hpp"

namespace tcob::gfx::ui {

template <std::derived_from<widget_container> T>
inline auto accordion::create_section(utf8_string const& name) -> std::shared_ptr<T>
{
    return create_section<T>(name, name);
}

template <std::derived_from<widget_container> T>
inline auto accordion::create_section(utf8_string const& name, utf8_string const& label) -> std::shared_ptr<T>
{
    force_redraw(get_name() + ": section created");

    widget::init wi {};
    wi.Form   = get_form();
    wi.Parent = this;
    wi.Name   = name;

    auto retValue {std::make_shared<T>(wi)};
    _sections.push_back(retValue);
    _sectionLabels.push_back(label);
    if (ActiveSectionIndex == -1) {
        ActiveSectionIndex = 0;
    }
    return retValue;
}
}
