// Copyright (c) 2021 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include <tcob/gfx/Animation.hpp>

#include <algorithm>

namespace tcob {
using namespace std::chrono_literals;

auto FrameAnimation::get_frame(MilliSeconds time) const -> std::string
{
    if (Duration <= 0ms || time < 0ms || Frames.empty()) {
        return "";
    }

    isize frameCount { Frames.size() };
    isize index {};
    if (Mode == AnimationPlaybackMode::Alternated || Mode == AnimationPlaybackMode::AlternatedLooped) {
        index = static_cast<isize>(time / (Duration / (frameCount * 2 - 1)));

        switch (Mode) {
        case AnimationPlaybackMode::Alternated:
            if (index > frameCount - 1) {
                index %= frameCount;
                index = frameCount - index - 2;
            }
            if (index >= frameCount) {
                index = 0;
            }
            break;
        case AnimationPlaybackMode::AlternatedLooped:
            index %= (frameCount * 2) - 2;
            if (index > frameCount - 1) {
                index %= frameCount;
                index = frameCount - index - 2;
            }
            break;
        default:
            break;
        }
    } else {
        index = static_cast<isize>(time / (Duration / frameCount));

        switch (Mode) {
        case AnimationPlaybackMode::Normal:
            if (index >= frameCount) {
                index = frameCount - 1;
            }
            break;
        case AnimationPlaybackMode::Reversed:
            if (index >= frameCount) {
                index = 0;
            } else {
                index = (frameCount - 1) - index;
            }
            break;
        case AnimationPlaybackMode::Looped:
            index %= frameCount;
            break;
        case AnimationPlaybackMode::ReversedLooped:
            index = (frameCount - 1 - index) % frameCount;
            break;
        default:
            break;
        }
    }

    return Frames[index];
}

////////////////////////////////////////////////////////////

FrameAnimationFunction::FrameAnimationFunction(MilliSeconds duration, FrameAnimation ani)
    : Duration { duration }
    , _animation { std::move(ani) }
{
}

auto FrameAnimationFunction::value(f32 elapsed) const -> std::string
{
    return _animation.get_frame(_animation.Duration * elapsed);
}
}