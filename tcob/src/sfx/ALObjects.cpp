// Copyright (c) 2021 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT
#include <tcob/sfx/ALObjects.hpp>

#include <cassert>

#include <AL/al.h>
#include <thread>

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

void Buffer::buffer_data(const i16* data, u64 frameCount, i32 channels, i32 freq) const
{
    i32 framesize { static_cast<i32>(frameCount * channels * sizeof(i16)) };
    assert(ID);
    alBufferData(ID,
        channels == 1 ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16,
        data, framesize, freq);
}

void Buffer::buffer_data(const f32* data, u64 frameCount, i32 channels, i32 freq) const
{
    i32 framesize { static_cast<i32>(frameCount * channels * sizeof(f32)) };
    assert(ID);
    alBufferData(ID,
        channels == 1 ? AL_FORMAT_MONO_FLOAT32 : AL_FORMAT_STEREO_FLOAT32,
        data, framesize, freq);
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

Source::Source(const Source& other)
{
    *this = other;
}

auto Source::operator=(const Source& other) -> Source&
{
    alGenSources(1, &ID);

    pitch(other.pitch());
    gain(other.gain());
    position(other.position());
    direction(other.direction());
    rolloff_factor(other.rolloff_factor());
    source_relatvie(other.source_relatvie());
    looping(other.looping());

    return *this;
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

void Source::pause()
{
    assert(ID);
    alSourcePause(ID);
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
    value = std::clamp(value, 0.5f, 2.0f);
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
    value = std::clamp(value, 0.0f, 1.0f);
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

auto Source::sec_offset() const -> f32
{
    assert(ID);
    f32 retValue { 0 };
    alGetSourcef(ID, AL_SEC_OFFSET, &retValue);
    return retValue;
}

void Source::sec_offset(f32 value) const
{
    assert(ID);
    alSourcef(ID, AL_SEC_OFFSET, value);
}

auto Source::buffers_queued() const -> i32
{
    assert(ID);
    i32 retValue { 0 };
    alGetSourcei(ID, AL_BUFFERS_QUEUED, &retValue);
    return retValue;
}

auto Source::buffers_processed() const -> i32
{
    assert(ID);
    i32 retValue { 0 };
    alGetSourcei(ID, AL_BUFFERS_PROCESSED, &retValue);
    return retValue;
}

auto Source::looping() const -> bool
{
    assert(ID);
    i32 retValue { 0 };
    alGetSourcei(ID, AL_LOOPING, &retValue);
    return retValue;
}

void Source::looping(bool value) const
{
    assert(ID);
    alSourcei(ID, AL_LOOPING, value);
}

auto Source::state() const -> AudioState
{
    assert(ID);
    i32 retValue { 0 };
    alGetSourcei(ID, AL_SOURCE_STATE, &retValue);
    switch (retValue) {
    case AL_INITIAL:
        return AudioState::Initial;
    case AL_PLAYING:
        return AudioState::Playing;
    case AL_STOPPED:
        return AudioState::Stopped;
    case AL_PAUSED:
        return AudioState::Paused;
    default:
        return AudioState::Initial;
    }
}

void Source::queue_buffers(const u32* buffers, i32 bufferCount)
{
    assert(ID);
    alSourceQueueBuffers(ID, bufferCount, buffers);
}

auto Source::unqueue_buffers(i32 bufferCount) -> std::vector<u32>
{
    if (bufferCount > 0) {
        assert(ID);
        std::vector<u32> retValue(bufferCount);
        alSourceUnqueueBuffers(ID, bufferCount, retValue.data());
        return retValue;
    }

    return {};
}

}