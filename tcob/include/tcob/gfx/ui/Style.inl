// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "Style.hpp"

namespace tcob::gfx::ui {

template <std::derived_from<style_base> T>
inline auto style_collection::create(string const& name, style_flags flags, style_attributes const& attribs) -> std::shared_ptr<T>
{
    std::shared_ptr<T> retValue {std::make_shared<T>()};
    _styles.emplace_back(name, flags, attribs, retValue);
    return retValue;
}

template <std::derived_from<widget> T>
inline auto style_collection::create(string const& name, style_flags flags, style_attributes const& attribs) -> std::shared_ptr<typename T::style>
{
    return create<typename T::style>(name, flags, attribs);
}

}
