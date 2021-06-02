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

auto AudioSource::volume() const -> f32
{
    return _source->gain();
}

void AudioSource::volume(f32 vol) const
{
    _source->gain(vol);
}

auto AudioSource::source() const -> al::Source*
{
    return _source.get();
}

}