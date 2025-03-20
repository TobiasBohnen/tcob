// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/audio/Recording.hpp"

#include <cassert>
#include <memory>

#include "AudioStream.hpp"

#include "tcob/audio/AudioSystem.hpp"
#include "tcob/audio/Buffer.hpp"
#include "tcob/core/ServiceLocator.hpp"

namespace tcob::audio {

recording::recording()
    : _output {locate_service<system>().create_input()}
{
}

recording::~recording()
{
}

void recording::start()
{
    if (_output->is_bound()) { return; }
    _output->bind();
}

auto recording::stop() -> buffer
{
    _output->unbind();

    auto const data {_output->get()};
    _output->clear();
    buffer::information const info {.Channels = 1, .SampleRate = RECORDING_SAMPLE_RATE, .FrameCount = static_cast<i64>(data.size() / sizeof(f32))};
    return buffer::Create(info, data);
}

}
