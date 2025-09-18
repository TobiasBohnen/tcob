// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <memory>

#include "tcob/core/Common.hpp"
#include "tcob/core/Interfaces.hpp"
#include "tcob/core/Signal.hpp"
#include "tcob/core/easing/Tween.hpp"
#include "tcob/gfx/animation/Animation.hpp"

namespace tcob::ui {
////////////////////////////////////////////////////////////

class TCOB_API widget_tweener : public non_copyable {
public:
    widget_tweener() = default;

    signal<> Changed;

    void start(f32 toValue, milliseconds delay);
    void reset(f32 value);

    void update(milliseconds deltaTime);

    auto current_value() const -> f32;
    auto target_value() const -> f32;

private:
    void set_value(f32 value);

    std::unique_ptr<linear_tween<f32>> _tween;
    f32                                _currentValue {0.0f};
    f32                                _targetValue {0.0f};
};

////////////////////////////////////////////////////////////

class TCOB_API animation_tweener {
public:
    animation_tweener() = default;

    signal<string const> Changed;

    void start(gfx::frame_animation const& ani, playback_mode mode);
    void stop();

    void update(milliseconds deltaTime);

private:
    std::unique_ptr<gfx::frame_animation_tween> _tween;
};

}
