// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "Panel.hpp"

#include <cassert>
#include <memory>

#include "tcob/gfx/ui/UI.hpp"

namespace tcob::gfx::ui {

template <std::derived_from<layout> T>
inline auto panel::create_layout(auto&&... args) -> std::shared_ptr<T>
{
    force_redraw(this->name() + ": layout created");
    _layout = std::make_shared<T>(this, std::move(args)...);
    return std::static_pointer_cast<T>(_layout);
}

template <std::derived_from<layout> T>
inline auto panel::get_layout() -> std::shared_ptr<T>
{
#if defined(TCOB_DEBUG)
    auto dp {std::dynamic_pointer_cast<T>(_layout)};
    assert(dp);
    return dp;
#else
    return std::static_pointer_cast<T>(_layout);
#endif
}
}
