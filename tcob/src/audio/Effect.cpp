// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/audio/Effect.hpp"

#include <cmath>
#include <vector>

#include "tcob/audio/Buffer.hpp"

namespace tcob::audio {

delay_effect::delay_effect(seconds delayTime, f32 feedback, f32 mix)
    : _delayTime {delayTime}
    , _feedback {feedback}
    , _mix {mix}
{
}

auto delay_effect::operator()(buffer const& buf) const -> buffer
{
    auto const  inData {buf.data()};
    auto const& info {buf.info()};
    auto const  inputFrames {static_cast<usize>(info.FrameCount)};
    auto const  delayFrames {static_cast<usize>(_delayTime.count() * info.Specs.SampleRate)};
    auto const  outputFrames {inputFrames + delayFrames};
    i32 const   channels {info.Specs.Channels};

    std::vector<f32> outData(outputFrames * channels, 0.0f);

    for (i32 ch {0}; ch < channels; ++ch) {
        std::vector<f32> echo(outputFrames, 0.0f);

        for (usize frame {0}; frame < outputFrames; ++frame) {
            f32 const dry {(frame < inputFrames) ? inData[(frame * channels) + ch] : 0.0f};
            f32       delayed {0.0f};

            if (frame >= delayFrames) {
                delayed = echo[frame - delayFrames];
            }

            echo[frame] = dry + (_feedback * delayed);

            usize const idx {(frame * channels) + ch};
            outData[idx] = ((1.0f - _mix) * dry) + (_mix * echo[frame]);
        }
    }

    return buffer::Create(info.Specs, outData);
}

////////////////////////////////////////////////////////////

pitch_shift_effect::pitch_shift_effect(f32 pitchFactor)
    : _pitchFactor {pitchFactor}
{
}

auto pitch_shift_effect::operator()(buffer const& buf) const -> buffer
{
    auto const  inData {buf.data()};
    auto const& info {buf.info()};
    auto const  outputFrames {static_cast<usize>(std::ceil(info.FrameCount / _pitchFactor))};
    i32 const   channels {info.Specs.Channels};

    std::vector<f32> outData(outputFrames * channels, 0.0f);

    for (i32 ch {0}; ch < channels; ++ch) {
        for (usize frame {0}; frame < outputFrames; ++frame) {
            f32 const   srcIdx {frame * _pitchFactor};
            usize const idx0 {static_cast<usize>(std::floor(srcIdx))};
            usize const idx1 {(idx0 + 1 < static_cast<usize>(info.FrameCount)) ? idx0 + 1 : idx0};

            f32 const sample0 {inData[(idx0 * channels) + ch]};
            f32 const sample1 {inData[(idx1 * channels) + ch]};

            f32 const frac {srcIdx - static_cast<f32>(idx0)};

            outData[(frame * channels) + ch] = sample0 + ((sample1 - sample0) * frac);
        }
    }

    return buffer::Create(info.Specs, outData);
}

}
