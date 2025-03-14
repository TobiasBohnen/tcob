// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <memory>

#include "tcob/core/Interfaces.hpp"
#include "tcob/core/Signal.hpp"
#include "tcob/gfx/animation/Tween.hpp"

namespace tcob::ui {
////////////////////////////////////////////////////////////

class TCOB_API widget_tweener : public non_copyable { // TODO: improve or remove
public:
    widget_tweener();

    signal<> Changed;

    void start(f32 toValue, milliseconds delay);

    void update(milliseconds deltaTime);

    auto current_value() const -> f32;
    auto target_value() const -> f32;
    void reset(f32 value);

private:
    void set_value(f32 value);

    std::unique_ptr<gfx::linear_tween<f32>> _tween;
    f32                                     _currentValue {0.0f};
    f32                                     _targetValue {0.0f};
};

}
