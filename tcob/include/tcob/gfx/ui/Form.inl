// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "Form.hpp"

namespace tcob::gfx::ui {

template <std::derived_from<widget_container> T>
inline auto form::create_container(dock_style dock, string const& name) -> std::shared_ptr<T>
{
    return _layout.create_widget<T>(dock, name);
}

template <std::derived_from<tooltip> T>
inline auto form::create_tooltip(string const& name) -> std::shared_ptr<T>
{
    widget::init wi {};
    wi.Form   = this;
    wi.Parent = nullptr;
    wi.Name   = name;

    return std::make_shared<T>(wi);
}

template <SubmitTarget Target>
inline void form::submit(Target& target)
{
    auto widgets {get_all_widgets()};
    for (auto* widget : widgets) {
        auto const props {widget->attributes()};
        auto const name {widget->name()};
        if (!props.empty() && !name.empty()) {
            target[name] = props;
        }
    }
}

}
