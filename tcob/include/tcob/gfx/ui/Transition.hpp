// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

namespace tcob::gfx::ui {
////////////////////////////////////////////////////////////

template <typename T>
class transition_context {
public:
    void start(T* target, milliseconds duration);
    void reset(T* target);

    void update(milliseconds deltaTime);

    auto is_active() const -> bool;
    auto current_style() const -> T*;

    template <typename S>
    void update_style(S& style);

private:
    T* _currentStyle {nullptr};
    T* _targetStyle {nullptr};
    T* _oldStyle {nullptr};

    milliseconds _duration {};
    milliseconds _currentTime {};
};

}

#include "Transition.inl"
