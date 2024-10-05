// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/audio/AudioSystem.hpp"

#include <AL/al.h>
#include <AL/alc.h>

namespace tcob::audio {

system::system()
    : _device {alcOpenDevice(nullptr)}
    , _context {alcCreateContext(_device, nullptr)}
{
    alcMakeContextCurrent(_context);
}

system::~system()
{
    alcMakeContextCurrent(nullptr);
    alcDestroyContext(_context);
    alcCloseDevice(_device);
}

}
