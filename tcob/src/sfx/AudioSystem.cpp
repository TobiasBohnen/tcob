// Copyright (c) 2021 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include <tcob/sfx/AudioSystem.hpp>

#include <AL/al.h>
#include <AL/alc.h>

namespace tcob {

AudioSystem::AudioSystem()
{
    _device = alcOpenDevice(NULL);
    _context = alcCreateContext(_device, NULL);
    alcMakeContextCurrent(_context);
}

AudioSystem::~AudioSystem()
{
    alcMakeContextCurrent(NULL);
    alcDestroyContext(_context);
    alcCloseDevice(_device);
}

}
