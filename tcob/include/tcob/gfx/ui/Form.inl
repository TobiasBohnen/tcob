// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "Form.hpp"

#include <memory>
#include <vector>

#include "tcob/core/Rect.hpp"
#include "tcob/gfx/ui/UI.hpp"
#include "tcob/gfx/ui/widgets/Widget.hpp"

namespace tcob::ui {

////////////////////////////////////////////////////////////

template <std::derived_from<layout> Layout>
inline form<Layout>::form(form_init const& init, auto&&... layoutArgs)
    : form_base {init.Name, rect_f {init.Bounds}}
    , _layout {this, layoutArgs...}
{
    _layout.Changed.connect([&]() { request_redraw("Layout changed"); });
}

template <std::derived_from<layout> Layout>
template <std::derived_from<widget_container> T>
inline auto form<Layout>::create_container(auto&&... args) -> std::shared_ptr<T>
{
    return _layout.template create_widget<T>(args...);
}

template <std::derived_from<layout> Layout>
inline auto form<Layout>::containers() const -> std::vector<std::shared_ptr<widget>> const&
{
    return _layout.widgets();
}

template <std::derived_from<layout> Layout>
inline void form<Layout>::remove_container(widget* widget)
{
    _layout.remove(widget);
}

template <std::derived_from<layout> Layout>
inline void form<Layout>::clear_containers()
{
    _layout.clear();
}

template <std::derived_from<layout> Layout>
inline void form<Layout>::apply_layout()
{
    _layout.apply(Bounds->Size);
}

template <std::derived_from<layout> Layout>
inline auto form<Layout>::get_layout() -> layout*
{
    return &_layout;
}

template <std::derived_from<layout> Layout>
inline auto form<Layout>::get_layout() const -> layout const*
{
    return &_layout;
}

////////////////////////////////////////////////////////////

template <std::derived_from<tooltip> T>
inline auto form_base::create_tooltip(string const& name) -> std::shared_ptr<T>
{
    widget::init const wi {
        .Form   = this,
        .Parent = nullptr,
        .Name   = name,
    };

    auto retValue {std::make_shared<T>(wi)};
    _tooltips.push_back(retValue);
    return retValue;
}

template <SubmitTarget Target>
inline void form_base::submit(Target& target)
{
    auto widgets {all_widgets()};
    for (auto* widget : widgets) {
        auto const props {widget->attributes()};
        auto const name {widget->name()};
        if (!props.empty() && !name.empty()) {
            target[name] = props;
        }
    }
}
}
