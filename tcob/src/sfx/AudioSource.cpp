// Copyright (c) 2021 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include <tcob/sfx/AudioSource.hpp>

namespace tcob {

AudioSource::AudioSource()
    : _source { std::make_unique<al::Source>() }
{
}

AudioSource::~AudioSource()
{
}

void AudioSource::restart()
{
    stop();
    start(_source->looping());
}

void AudioSource::toggle_pause()
{
    if (_source->state() == AudioState::Paused) {
        _source->play();
    } else if (_source->state() == AudioState::Playing) {
        _source->pause();
    }
}

auto AudioSource::volume() const -> f32
{
    return _source->gain();
}

void AudioSource::volume(f32 vol) const
{
    _source->gain(vol);
}

auto AudioSource::state() const -> AudioState
{
    return _source->state();
}

auto AudioSource::source() const -> al::Source*
{
    return _source.get();
}

}