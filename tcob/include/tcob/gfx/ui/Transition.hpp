// Copyright (c) 2026 Tobias Bohnen
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
    void try_start(T const* target, milliseconds duration, milliseconds step);
    void reset(T const* target);

    auto update(milliseconds deltaTime) -> bool;

    template <typename S>
    void apply(S& style);

private:
    auto is_active() const -> bool;

    T const* _targetStyle {nullptr};
    T const* _sourceStyle {nullptr};

    milliseconds _duration {};
    milliseconds _currentTime {};
    milliseconds _accStep {};
    milliseconds _step {};
};

}

#include "Transition.inl"
