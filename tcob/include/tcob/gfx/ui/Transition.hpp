// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

namespace tcob::gfx::ui {
////////////////////////////////////////////////////////////

template <typename T>
class transition {
public:
    void start(T const* target, milliseconds duration);
    void reset(T const* target);

    void update(milliseconds deltaTime);
    auto is_active() const -> bool;

    auto current_style() const -> T const*;

    template <typename S>
    void update_style(S& style);

private:
    T const* _currentStyle {nullptr};
    T const* _targetStyle {nullptr};
    T const* _oldStyle {nullptr};

    milliseconds _duration {};
    milliseconds _currentTime {};
};

}

#include "Transition.inl"
