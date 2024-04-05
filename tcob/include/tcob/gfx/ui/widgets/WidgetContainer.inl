// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "WidgetContainer.hpp"

namespace tcob::gfx::ui {

template <SubmitTarget Target>
inline void widget_container::submit(Target& target)
{
    auto widgets {get_widgets()};
    for (auto const& widget : widgets) {
        auto const props {widget->get_properties()};
        auto const name {widget->get_name()};
        if (!props.empty() && !name.empty()) {
            target[name] = props;
        }
    }
}

}
