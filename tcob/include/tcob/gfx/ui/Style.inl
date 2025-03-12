// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "Style.hpp"

#include <memory>

#include "tcob/gfx/ui/UI.hpp"

namespace tcob::ui {

template <std::derived_from<style> T>
inline auto style_collection::create(string const& name, style_flags flags, style_attributes const& attribs) -> std::shared_ptr<T>
{
    std::shared_ptr<T> retValue {std::make_shared<T>()};
    _styles[name].emplace_back(flags, attribs, retValue);
    return retValue;
}

template <std::derived_from<widget> T>
inline auto style_collection::create(string const& name, style_flags flags, style_attributes const& attribs) -> std::shared_ptr<typename T::style>
{
    return create<typename T::style>(name, flags, attribs);
}

}
