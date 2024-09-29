// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT
#include "ALObjects.hpp"

#include <cassert>

namespace tcob::audio::al {
al_buffer::al_buffer()
{
    assert(!_id);
    alGenBuffers(1, &_id);
}

al_buffer::~al_buffer()
{
    if (_id) {
        alDeleteBuffers(1, &_id);
        _id = 0;
    }
}

void al_buffer::buffer_data(std::span<i16 const> data, i32 channels, i32 freq) const
{
    assert(_id);
    alBufferData(
        _id,
        channels == 1 ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16,
        data.data(), static_cast<i32>(data.size_bytes()), freq);
}

void al_buffer::buffer_data(std::span<f32 const> data, i32 channels, i32 freq) const
{
    assert(_id);
    alBufferData(
        _id,
        channels == 1 ? AL_FORMAT_MONO_FLOAT32 : AL_FORMAT_STEREO_FLOAT32,
        data.data(), static_cast<i32>(data.size_bytes()), freq);
}

auto al_buffer::get_frequency() const -> i32
{
    assert(_id);
    i32 retValue {0};
    alGetBufferi(_id, AL_FREQUENCY, &retValue);
    return retValue;
}

auto al_buffer::get_size() const -> i32
{
    assert(_id);
    i32 retValue {0};
    alGetBufferi(_id, AL_SIZE, &retValue);
    return retValue;
}

auto al_buffer::get_bits() const -> i32
{
    assert(_id);
    i32 retValue {0};
    alGetBufferi(_id, AL_BITS, &retValue);
    return retValue;
}

auto al_buffer::get_channels() const -> i32
{
    assert(_id);
    i32 retValue {0};
    alGetBufferi(_id, AL_CHANNELS, &retValue);
    return retValue;
}

auto al_buffer::get_id() const -> u32
{
    return _id;
}

auto al_buffer::GetSize(u32 bufferID) -> i32
{
    assert(bufferID);
    i32 retValue {0};
    alGetBufferi(bufferID, AL_SIZE, &retValue);
    return retValue;
}

////////////////////////////////////////////////////////////

al_source::al_source()
{
    alGenSources(1, &_id);
    alSourcef(_id, AL_PITCH, 1);
    alSourcef(_id, AL_GAIN, 1);

    alSource3f(_id, AL_POSITION, 0.0f, 0.0f, 0.0f);
    alSource3f(_id, AL_VELOCITY, 0.0f, 0.0f, 0.0f);
    alSource3f(_id, AL_DIRECTION, 0.0f, 0.0f, 0.0f);
    alSourcef(_id, AL_ROLLOFF_FACTOR, 0.0f);
    alSourcei(_id, AL_SOURCE_RELATIVE, false);
}

al_source::~al_source()
{
    if (_id) {
        alDeleteSources(1, &_id);
        _id = 0;
    }
}

al_source::al_source(al_source const& other) noexcept
{
    *this = other;
}

auto al_source::operator=(al_source const& other) noexcept -> al_source&
{
    if (this != &other) {
        alGenSources(1, &_id);

        set_pitch(other.get_pitch());
        set_gain(other.get_gain());
        set_position(other.get_position());
        set_direction(other.get_direction());
        set_rolloff_factor(other.get_rolloff_factor());
        set_source_relative(other.get_source_relative());
        set_looping(other.is_looping());
    }

    return *this;
}

void al_source::play() const
{
    assert(_id);
    alSourcePlay(_id);
}

void al_source::stop() const
{
    assert(_id);
    alSourceStop(_id);
}

void al_source::pause() const
{
    assert(_id);
    alSourcePause(_id);
}

void al_source::set_buffer(u32 bufferID) const
{
    assert(_id);
    alSourcei(_id, AL_BUFFER, static_cast<i32>(bufferID));
}

auto al_source::get_pitch() const -> f32
{
    assert(_id);
    f32 retValue {0};
    alGetSourcef(_id, AL_PITCH, &retValue);
    return retValue;
}

void al_source::set_pitch(f32 value) const
{
    assert(_id);
    alSourcef(_id, AL_PITCH, std::clamp(value, 0.5f, 2.0f));
}

auto al_source::get_gain() const -> f32
{
    assert(_id);
    f32 retValue {0};
    alGetSourcef(_id, AL_GAIN, &retValue);
    return retValue;
}

void al_source::set_gain(f32 value) const
{
    assert(_id);
    alSourcef(_id, AL_GAIN, std::clamp(value, 0.0f, 1.0f));
}

auto al_source::get_position() const -> std::array<f32, 3>
{
    assert(_id);
    std::array<f32, 3> retValue {};
    alGetSource3f(_id, AL_POSITION, &retValue[0], &retValue[1], &retValue[2]);
    return retValue;
}

void al_source::set_position(std::array<f32, 3> const& value) const
{
    assert(_id);
    alSource3f(_id, AL_POSITION, value[0], value[1], value[2]);
}

auto al_source::get_direction() const -> std::array<f32, 3>
{
    assert(_id);
    std::array<f32, 3> retValue {};
    alGetSource3f(_id, AL_DIRECTION, &retValue[0], &retValue[1], &retValue[2]);
    return retValue;
}

void al_source::set_direction(std::array<f32, 3> const& value) const
{
    assert(_id);
    alSource3f(_id, AL_DIRECTION, value[0], value[1], value[2]);
}

auto al_source::get_rolloff_factor() const -> f32
{
    assert(_id);
    f32 retValue {0};
    alGetSourcef(_id, AL_ROLLOFF_FACTOR, &retValue);
    return retValue;
}

void al_source::set_rolloff_factor(f32 value) const
{
    assert(_id);
    alSourcef(_id, AL_ROLLOFF_FACTOR, value);
}

auto al_source::get_source_relative() const -> bool
{
    assert(_id);
    i32 retValue {0};
    alGetSourcei(_id, AL_SOURCE_RELATIVE, &retValue);
    return retValue;
}

void al_source::set_source_relative(bool value) const
{
    assert(_id);
    alSourcei(_id, AL_SOURCE_RELATIVE, value);
}

auto al_source::get_sec_offset() const -> f32
{
    assert(_id);
    f32 retValue {0};
    alGetSourcef(_id, AL_SEC_OFFSET, &retValue);
    return retValue;
}

void al_source::set_sec_offset(f32 value) const
{
    assert(_id);
    alSourcef(_id, AL_SEC_OFFSET, value);
}

auto al_source::get_buffers_queued() const -> i32
{
    assert(_id);
    i32 retValue {0};
    alGetSourcei(_id, AL_BUFFERS_QUEUED, &retValue);
    return retValue;
}

auto al_source::get_buffers_processed() const -> i32
{
    assert(_id);
    i32 retValue {0};
    alGetSourcei(_id, AL_BUFFERS_PROCESSED, &retValue);
    return retValue;
}

auto al_source::is_looping() const -> bool
{
    assert(_id);
    i32 retValue {0};
    alGetSourcei(_id, AL_LOOPING, &retValue);
    return retValue;
}

void al_source::set_looping(bool value) const
{
    assert(_id);
    alSourcei(_id, AL_LOOPING, value);
}

auto al_source::get_status() const -> playback_status
{
    assert(_id);
    i32 retValue {0};
    alGetSourcei(_id, AL_SOURCE_STATE, &retValue);
    switch (retValue) {
    case AL_INITIAL:
    case AL_STOPPED:
        return playback_status::Stopped;
    case AL_PLAYING:
        return playback_status::Running;
    case AL_PAUSED:
        return playback_status::Paused;
    default:
        return playback_status::Stopped;
    }
}

void al_source::queue_buffers(u32 const* buffers, i32 bufferCount) const
{
    assert(_id);
    alSourceQueueBuffers(_id, bufferCount, buffers);
}

auto al_source::unqueue_buffers(i32 bufferCount) const -> std::vector<u32>
{
    if (bufferCount <= 0) {
        return {};
    }

    assert(_id);
    std::vector<u32> retValue(static_cast<usize>(bufferCount));
    alSourceUnqueueBuffers(_id, bufferCount, retValue.data());
    return retValue;
}

auto al_source::get_id() const -> u32
{
    return _id;
}

}
