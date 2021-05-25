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

void Buffer::buffer_data(i32 channels, const void* data, i32 size, i32 freq) const
{
    assert(ID);
    alBufferData(ID,
        channels == 1 ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16,
        data, size, freq);
}

auto Buffer::frequency() const -> i32
{
    assert(ID);
    i32 retValue { 0 };
    alGetBufferi(ID, AL_FREQUENCY, &retValue);
    return retValue;
}

auto Buffer::size() const -> i32
{
    assert(ID);
    i32 retValue { 0 };
    alGetBufferi(ID, AL_SIZE, &retValue);
    return retValue;
}

auto Buffer::bits() const -> i32
{
    assert(ID);
    i32 retValue { 0 };
    alGetBufferi(ID, AL_BITS, &retValue);
    return retValue;
}

auto Buffer::channels() const -> i32
{
    assert(ID);
    i32 retValue { 0 };
    alGetBufferi(ID, AL_CHANNELS, &retValue);
    return retValue;
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

auto Source::pitch() const -> f32
{
    assert(ID);
    f32 retValue { 0 };
    alGetSourcef(ID, AL_PITCH, &retValue);
    return retValue;
}

void Source::pitch(f32 value) const
{
    assert(ID);
    alSourcef(ID, AL_PITCH, value);
}

auto Source::gain() const -> f32
{
    assert(ID);
    f32 retValue { 0 };
    alGetSourcef(ID, AL_GAIN, &retValue);
    return retValue;
}

void Source::gain(f32 value) const
{
    assert(ID);
    alSourcef(ID, AL_GAIN, value);
}

auto Source::position() const -> std::array<f32, 3>
{
    assert(ID);
    std::array<f32, 3> retValue {};
    alGetSource3f(ID, AL_POSITION, &retValue[0], &retValue[1], &retValue[2]);
    return retValue;
}

void Source::position(const std::array<f32, 3>& value) const
{
    assert(ID);
    alSource3f(ID, AL_POSITION, value[0], value[1], value[2]);
}

auto Source::direction() const -> std::array<f32, 3>
{
    assert(ID);
    std::array<f32, 3> retValue {};
    alGetSource3f(ID, AL_DIRECTION, &retValue[0], &retValue[1], &retValue[2]);
    return retValue;
}

void Source::direction(const std::array<f32, 3>& value) const
{
    assert(ID);
    alSource3f(ID, AL_DIRECTION, value[0], value[1], value[2]);
}

auto Source::rolloff_factor() const -> f32
{
    assert(ID);
    f32 retValue { 0 };
    alGetSourcef(ID, AL_ROLLOFF_FACTOR, &retValue);
    return retValue;
}

void Source::rolloff_factor(f32 value) const
{
    assert(ID);
    alSourcef(ID, AL_ROLLOFF_FACTOR, value);
}

auto Source::source_relatvie() const -> bool
{
    assert(ID);
    i32 retValue { 0 };
    alGetSourcei(ID, AL_SOURCE_RELATIVE, &retValue);
    return retValue;
}

void Source::source_relatvie(bool value) const
{
    assert(ID);
    alSourcei(ID, AL_SOURCE_RELATIVE, value);
}
}