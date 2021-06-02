// Copyright (c) 2021 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include <tcob/tcob_config.hpp>

#include <tcob/core/Automation.hpp>

namespace tcob {
enum class AnimationPlaybackMode {
    Normal,
    Reversed,
    Looped,
    ReversedLooped,
    Alternated,
    AlternatedLooped
};

////////////////////////////////////////////////////////////

struct FrameAnimation final {
public:
    auto get_frame(f64 time) const -> std::string;

    std::vector<std::string> Frames {};
    f64 Duration { 0 };
    AnimationPlaybackMode Mode { AnimationPlaybackMode::Normal };
};

////////////////////////////////////////////////////////////

struct FrameAnimationFunction final {
    using type = std::string;

    f64 Duration;

    FrameAnimationFunction(f64 duration, FrameAnimation ani);

    auto value(f32 elapsed) const -> std::string;

private:
    FrameAnimation _animation;
};
}