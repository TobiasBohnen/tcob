// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "Panel.hpp"

#include <cassert>
#include <memory>

#include "tcob/gfx/ui/Layout.hpp"
#include "tcob/gfx/ui/UI.hpp"

namespace tcob::ui {

template <std::derived_from<layout> T>
inline auto panel::create_layout(auto&&... args) -> T&
{
    request_redraw(this->name() + ": Layout created");
    _layout = std::make_unique<T>(this, std::move(args)...);
    _layout->Changed.connect([&]() { request_redraw("Layout changed"); });
    return *static_cast<T*>(_layout.get());
}

template <std::derived_from<layout> T>
inline auto panel::get_layout() -> T&
{
#if defined(TCOB_DEBUG)
    auto dp {dynamic_cast<T*>(_layout.get())};
    assert(dp);
    return *dp;
#else
    return *static_cast<T*>(_layout.get());
#endif
}
}
