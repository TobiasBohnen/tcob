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
    auto const  delayFrames {static_cast<usize>(_delayTime.count() * info.SampleRate)};
    auto const  outputFrames {inputFrames + delayFrames};

    std::vector<f32> outData(outputFrames * info.Channels, 0.0f);

    for (i32 ch {0}; ch < info.Channels; ++ch) {
        std::vector<f32> echo(outputFrames, 0.0f);

        for (usize frame {0}; frame < outputFrames; ++frame) {
            f32 const dry {(frame < inputFrames) ? inData[frame * info.Channels + ch] : 0.0f};
            f32       delayed {0.0f};

            if (frame >= delayFrames) {
                delayed = echo[frame - delayFrames];
            }

            echo[frame] = dry + _feedback * delayed;

            usize const idx {frame * info.Channels + ch};
            outData[idx] = (1.0f - _mix) * dry + _mix * echo[frame];
        }
    }

    buffer::information const outInfo {
        .Channels   = info.Channels,
        .SampleRate = info.SampleRate,
        .FrameCount = static_cast<i64>(outputFrames)};
    return buffer::Create(outInfo, outData);
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

    std::vector<f32> outData(outputFrames * info.Channels, 0.0f);

    for (i32 ch {0}; ch < info.Channels; ++ch) {
        for (usize frame {0}; frame < outputFrames; ++frame) {
            f32 const   srcIdx {frame * _pitchFactor};
            usize const idx0 {static_cast<usize>(std::floor(srcIdx))};
            usize const idx1 {(idx0 + 1 < static_cast<usize>(info.FrameCount)) ? idx0 + 1 : idx0};

            f32 const sample0 {inData[idx0 * info.Channels + ch]};
            f32 const sample1 {inData[idx1 * info.Channels + ch]};

            f32 const frac {srcIdx - static_cast<f32>(idx0)};

            outData[frame * info.Channels + ch] = sample0 + (sample1 - sample0) * frac;
        }
    }

    buffer::information const outInfo {
        .Channels   = info.Channels,
        .SampleRate = info.SampleRate, // Sample rate remains unchanged.
        .FrameCount = static_cast<i64>(outputFrames)};
    return buffer::Create(outInfo, outData);
}

} // namespace audio
