// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

namespace tcob::ui {
////////////////////////////////////////////////////////////

template <typename T>
class transition {
public:
    void try_start(T const* target, milliseconds duration);
    void reset(T const* target);

    void update(milliseconds deltaTime);
    auto is_active() const -> bool;

    template <typename S>
    void apply(S& style);

private:
    T const* _targetStyle {nullptr};
    T const* _sourceStyle {nullptr};

    milliseconds _duration {};
    milliseconds _currentTime {};
};

}

#include "Transition.inl"
