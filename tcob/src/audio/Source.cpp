// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/audio/Source.hpp"

#include <cassert>
#include <memory>

#include "AudioStream.hpp"

#include "tcob/audio/Audio.hpp"
#include "tcob/audio/AudioSystem.hpp"
#include "tcob/core/Common.hpp"
#include "tcob/core/ServiceLocator.hpp"

namespace tcob::audio {

source::source()
{
    Volume.Changed.connect([this](f32 val) {
        if (_output) { _output->set_volume(val); }
    });
    Volume(1.0f);
}

source::~source()
{
    if (!_output) { return; }
    _output->clear();
    _output->unbind();
}

auto source::status() const -> playback_status
{
    if (!_output) { return playback_status::Stopped; }
    if (!_output->is_bound()) {
        if (_output->available_bytes() > 0) {
            return playback_status::Paused;
        }
        return playback_status::Stopped;
    }
    if (_output->available_bytes() > 0) {
        return playback_status::Running;
    }
    return playback_status::Stopped;
}

void source::play()
{
    if (!_output) { return; }

    auto const stat {status()};
    if (stat == playback_status::Stopped) { // start if stopped
        if (on_start()) {
            if (!_output->is_bound()) {
                _output->bind();
            }
        }
    } else if (stat == playback_status::Paused) { // resume if paused
        resume();
    }
}

void source::stop()
{
    if (!_output) { return; }

    if (status() != playback_status::Stopped) { // stop if running or paused
        if (on_stop()) {
            _output->unbind();
        }
    }
}

void source::restart()
{
    stop();
    play();
}

void source::pause()
{
    if (!_output) { return; }

    if (status() == playback_status::Running) {
        _output->unbind();
    }
}

void source::resume()
{
    if (!_output) { return; }

    if (status() == playback_status::Paused) {
        _output->bind();
    }
}

void source::toggle_pause()
{
    status() == playback_status::Paused ? resume() : pause();
}

void source::create_output(specification const& info)
{
    _output = locate_service<system>().create_output(info);
    _output->set_volume(Volume);
}

auto source::get_output() -> detail::audio_stream&
{
    assert(_output);
    return *_output;
}

}
