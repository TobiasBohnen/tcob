// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include "tcob/audio/Audio.hpp"

namespace tcob::audio {
////////////////////////////////////////////////////////////

class TCOB_API effect_base {
public:
    effect_base()          = default;
    virtual ~effect_base() = default;

    auto virtual operator()(buffer const& img) const -> buffer = 0;
};

////////////////////////////////////////////////////////////

class TCOB_API delay_effect : public effect_base {
public:
    delay_effect(seconds delayTime, f32 feedback, f32 mix);

    auto operator()(buffer const& buf) const -> buffer override;

private:
    seconds _delayTime;
    f32     _feedback;
    f32     _mix;
};

////////////////////////////////////////////////////////////

class TCOB_API pitch_shift_effect : public effect_base {
public:
    explicit pitch_shift_effect(f32 pitchFactor);

    auto operator()(buffer const& buf) const -> buffer override;

private:
    f32 _pitchFactor;
};

}
