// Copyright (c) 2021 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT
#include <tcob/sfx/ALObjects.hpp>

#include <cassert>

#include <AL/al.h>

namespace tcob::al {
Buffer::Buffer()
{
    assert(!ID);
    alGenBuffers(1, &ID);
}

Buffer::~Buffer()
{
    if (ID) {
        alDeleteBuffers(1, &ID);
        ID = 0;
    }
}

void Buffer::buffer_data(i32 channels, const void* data, i32 size, i32 freq)
{
    assert(ID);
    alBufferData(ID,
        channels == 1 ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16,
        data, size, freq);
}

////////////////////////////////////////////////////////////

Source::Source()
{
    assert(!ID);
    alGenSources(1, &ID);
    alSourcef(ID, AL_PITCH, 1);
    alSourcef(ID, AL_GAIN, 1);

    alSource3f(ID, AL_POSITION, 0.0f, 0.0f, 0.0f);
    alSource3f(ID, AL_VELOCITY, 0.0f, 0.0f, 0.0f);
    alSource3f(ID, AL_DIRECTION, 0.0f, 0.0f, 0.0f);
    alSourcef(ID, AL_ROLLOFF_FACTOR, 0.0f);
    alSourcei(ID, AL_SOURCE_RELATIVE, false);
}

Source::~Source()
{
    if (ID) {
        alDeleteSources(1, &ID);
        ID = 0;
    }
}

void Source::play()
{
    assert(ID);
    alSourcePlay(ID);
}

void Source::stop()
{
    assert(ID);
    alSourceStop(ID);
}

void Source::buffer(u32 bufferID)
{
    assert(ID);
    alSourcei(ID, AL_BUFFER, bufferID);
}

}