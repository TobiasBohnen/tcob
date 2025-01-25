// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "WidgetContainer.hpp"

namespace tcob::gfx::ui {

template <SubmitTarget Target>
inline void widget_container::submit(Target& target)
{
    auto const props {attributes()};
    auto const n {name()};
    if (!props.empty() && !n.empty()) {
        target[n] = props;
    }

    for (auto const& widget : widgets()) {
        auto const wprops {widget->attributes()};
        auto const wname {widget->name()};
        if (!wprops.empty() && !wname.empty()) {
            target[wname] = wprops;
        }
    }
}

}
