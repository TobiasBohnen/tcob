// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "Accordion.hpp"

#include <memory>

#include "tcob/gfx/ui/UI.hpp"
#include "tcob/gfx/ui/component/Item.hpp"
#include "tcob/gfx/ui/widgets/WidgetContainer.hpp"

namespace tcob::ui {

template <std::derived_from<widget_container> T>
inline auto accordion::create_section(utf8_string const& name) -> T&
{
    return create_section<T>(name, {.Text = name, .Icon = {}, .UserData = {}});
}

template <std::derived_from<widget_container> T>
inline auto accordion::create_section(utf8_string const& name, item const& label) -> T&
{
    queue_redraw();

    widget::init const wi {
        .Form   = &form(),
        .Parent = this,
        .Name   = name,
    };

    auto  ptr {std::make_unique<T>(wi)};
    auto& retValue {*ptr};
    _sections.push_back(std::move(ptr));
    _sectionLabels.push_back(label);

    return retValue;
}
}
