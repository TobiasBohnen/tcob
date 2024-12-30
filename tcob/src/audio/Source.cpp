// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/audio/Source.hpp"

#include "ALObjects.hpp"

namespace tcob::audio {

source::source()
    : Volume {{[&]() { return _source->get_gain(); },
               [&](auto const& value) { _source->set_gain(value); }}}
    , _source {std::make_shared<audio::al::al_source>()}
{
    Volume(1.0f);
}

source::~source() = default;

source::source(source const& other) noexcept
    : source {}
{
    *this = other;
}

auto source::operator=(source const& other) noexcept -> source&
{
    if (this != &other) {
        _source = other._source;
        Volume  = other.Volume();
    }

    return *this;
}

auto source::status() const -> playback_status
{
    return _source->get_status();
}

auto source::is_looping() const -> bool
{
    return _source->is_looping();
}

void source::play(bool looping)
{
    if (status() == playback_status::Stopped) { // start if stopped
        if (on_start()) {
            _source->set_looping(looping);
        }
    } else if (status() == playback_status::Paused) { // resume if paused
        _source->set_looping(looping);
        resume();
    }
}

void source::stop()
{
    if (status() != playback_status::Stopped) { // stop if running or paused
        on_stop();
    }
}

void source::restart()
{
    stop();
    play(is_looping());
}

void source::pause()
{
    if (status() == playback_status::Running) {
        _source->pause();
    }
}

void source::resume()
{
    if (status() == playback_status::Paused) {
        _source->play();
    }
}

void source::toggle_pause()
{
    status() == playback_status::Paused ? resume() : pause();
}

auto source::get_impl() const -> audio::al::al_source*
{
    return _source.get();
}

}
