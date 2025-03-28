// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/audio/Source.hpp"

#include <algorithm>
#include <cassert>
#include <memory>
#include <span>
#include <vector>

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

    Panning(0.0f);
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
        if (_output->queued_bytes() > 0) { return playback_status::Paused; }
        return playback_status::Stopped;
    }
    if (_output->queued_bytes() > 0) { return playback_status::Running; }
    return playback_status::Stopped;
}

void source::play()
{
    if (!_output) { return; }

    auto const stat {status()};
    if (stat == playback_status::Stopped) { // start if stopped
        _output->clear();
        if (on_start()) {
            if (!_output->is_bound()) { _output->bind(); }
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
            _output->flush();
            _output->unbind();
            _output->clear();
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

void source::create_output()
{
    auto const spec {info()};
    if (!spec) { return; }

    _output = locate_service<system>().create_output(*spec);
    _output->set_volume(Volume);
    _canPan = (spec->Channels & 1) == 0;
}

void source::write_to_output(std::span<f32 const> data)
{
    assert(_output);

    if (_canPan && Panning != 0.0f) {
        f32 const        pan {std::clamp(Panning(), -1.0f, 1.0f)};
        std::vector<f32> buffer {data.begin(), data.end()};
        for (usize i {0}; i < data.size(); i += 2) {
            f32 const leftGain {(pan < 0) ? 1.0f : (1.0f - pan)};
            f32 const rightGain {(pan > 0) ? 1.0f : (1.0f + pan)};
            buffer[i + 0] = data[i + 0] * leftGain;
            buffer[i + 1] = data[i + 1] * rightGain;
        }
        _output->put(buffer);
        return;
    }

    _output->put(data);
}

void source::flush_output()
{
    assert(_output);
    _output->flush();
}

auto source::queued_bytes() const -> i32
{
    assert(_output);
    return _output->queued_bytes();
}

}
