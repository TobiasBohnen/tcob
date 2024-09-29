// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/audio/SpeechGenerator.hpp"

#include <speech/speech.h>

namespace tcob::audio {

constexpr i32 sampleRate {44100};
constexpr i32 channels {2};

auto speech_generator::create_buffer [[nodiscard]] (std::string const& text) -> buffer
{
    i32   frames {0};
    auto* data {speech_gen(&frames, text.c_str(), nullptr)};

    std::vector<f32> databuf;
    databuf.resize(static_cast<usize>(frames * channels));
    for (i32 i {0}; i < frames * channels; ++i) { databuf[static_cast<usize>(i)] = data[i] / 32768.0f; }
    speech_free(data, nullptr);

    return buffer::Create({.Channels = channels, .SampleRate = sampleRate, .FrameCount = frames}, databuf);
}

auto speech_generator::create_sound [[nodiscard]] (std::string const& text) -> sound
{
    auto audioData {create_buffer(text)};
    return sound {audioData};
}

////////////////////////////////////////////////////////////

}
