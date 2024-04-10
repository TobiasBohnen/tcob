// Copyright (c) 2023 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "Widget.hpp"

namespace tcob::gfx::ui {

template <std::derived_from<style_base> T>
inline auto widget::get_style() const -> T*
{
    return static_cast<T*>(_style.get());
}

template <std::derived_from<style_base> T>
inline auto widget::get_sub_style(string const& styleClass, flags flags) const -> T*
{
    return static_cast<T*>(get_styles().get(styleClass, flags, get_attributes()).get());
}

}
