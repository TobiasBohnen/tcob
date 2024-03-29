// Copyright (c) 2023 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "Panel.hpp"

namespace tcob::gfx::ui {

template <std::derived_from<layout> T>
inline auto panel::create_layout(auto&&... args) -> std::shared_ptr<T>
{
    force_redraw(get_name() + ": layout created");
    _layout = std::make_shared<T>(this, std::move(args)...);
    return std::dynamic_pointer_cast<T>(_layout);
}

template <std::derived_from<layout> T>
inline auto panel::get_layout() -> std::shared_ptr<T>
{
    return std::dynamic_pointer_cast<T>(_layout);
}
}
