// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/audio/Recording.hpp"

#include <cassert>
#include <memory>

#include "tcob/audio/Audio.hpp"
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
    _output->flush();
    auto data {_output->get()};
    _output->clear();
    return buffer::Create({.Channels = 1, .SampleRate = RECORDING_SAMPLE_RATE}, data);
}

}
