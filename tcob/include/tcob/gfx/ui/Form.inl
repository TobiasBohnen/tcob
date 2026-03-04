// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "Form.hpp"

#include <memory>
#include <span>
#include <vector>

#include "tcob/core/Rect.hpp"
#include "tcob/gfx/ui/UI.hpp"
#include "tcob/gfx/ui/widgets/Widget.hpp"

namespace tcob::ui {

////////////////////////////////////////////////////////////

template <DerivedFrom<layout> Layout>
inline form<Layout>::form(form_init const& init, auto&&... layoutArgs)
    : form_base {init.Name, rect_f {init.Bounds}}
    , _layout {this, layoutArgs...}
{
    _layout.Changed.connect([&] { queue_redraw(); });
}

template <DerivedFrom<layout> Layout>
template <DerivedFrom<widget_container> T>
inline auto form<Layout>::create_container(auto&&... args) -> T&
{
    return _layout.template create_widget<T>(args...);
}

template <DerivedFrom<layout> Layout>
inline auto form<Layout>::containers() const -> std::span<std::unique_ptr<widget> const>
{
    return _layout.widgets();
}

template <DerivedFrom<layout> Layout>
inline void form<Layout>::remove_container(widget* widget)
{
    _layout.remove(widget);
}

template <DerivedFrom<layout> Layout>
inline void form<Layout>::clear_containers()
{
    _layout.clear();
}

template <DerivedFrom<layout> Layout>
inline void form<Layout>::apply_layout()
{
    _layout.apply(Bounds->Size);
}

template <DerivedFrom<layout> Layout>
inline auto form<Layout>::get_layout() -> layout*
{
    return &_layout;
}

template <DerivedFrom<layout> Layout>
inline auto form<Layout>::get_layout() const -> layout const*
{
    return &_layout;
}

template <DerivedFrom<layout> Layout>
inline auto form<Layout>::allows_move() const -> bool
{
    return _layout.allows_move();
}

template <DerivedFrom<layout> Layout>
inline auto form<Layout>::allows_resize() const -> bool
{
    return _layout.allows_resize();
}

////////////////////////////////////////////////////////////

template <DerivedFrom<tooltip> T>
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

template <DerivedFrom<modal_dialog> T>
inline auto form_base::create_modal_dialog(string const& name) -> std::shared_ptr<T>
{
    widget::init const wi {
        .Form   = this,
        .Parent = nullptr,
        .Name   = name,
    };

    return std::make_shared<T>(wi);
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
