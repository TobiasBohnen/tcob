// Copyright (c) 2021 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include <tcob/sfx/Sound.hpp>

#define DR_FLAC_NO_STDIO
#include <dr_libs/dr_flac.h>
#define DR_MP3_NO_STDIO
#include <dr_libs/dr_mp3.h>
#define DR_WAV_NO_STDIO
#include <dr_libs/dr_wav.h>
#define STB_VORBIS_NO_STDIO
#define STB_VORBIS_HEADER_ONLY
#include <stb/stb_vorbis.c>

#include <tcob/core/io/FileStream.hpp>

namespace tcob {

Sound::Sound()
    : _buffer { std::make_shared<al::Buffer>() }
{
}

Sound::~Sound()
{
    stop();
    source()->buffer(0);
}

auto Sound::load(const std::string& filename) -> bool
{
    if (FileSystem::is_file(filename)) {
        InputFileStream stream { filename };
        auto buffer { stream.read_all() };
        std::string ext { FileSystem::extension(filename) };

        f32* audioData { nullptr };
        u64 frameCount { 0 };
        u32 channels { 0 }, sampleRate { 0 };

        if (ext == ".wav") {
            audioData = drwav_open_memory_and_read_pcm_frames_f32(buffer.data(), buffer.size(), &channels, &sampleRate, &frameCount, nullptr);
        } else if (ext == ".flac") {
            audioData = drflac_open_memory_and_read_pcm_frames_f32(buffer.data(), buffer.size(), &channels, &sampleRate, &frameCount, nullptr);
        } else if (ext == ".mp3") {
            drmp3_config config {};
            audioData = drmp3_open_memory_and_read_pcm_frames_f32(buffer.data(), buffer.size(), &config, &frameCount, nullptr);
            channels = config.channels;
            sampleRate = config.sampleRate;
        } else if (ext == ".ogg") {
            i32 c, sr;
            frameCount = stb_vorbis_decode_memory_float(reinterpret_cast<u8*>(buffer.data()), static_cast<i32>(buffer.size()), &c, &sr, &audioData);
            channels = static_cast<u32>(c);
            sampleRate = static_cast<u32>(sr);
        }

        if (audioData) {
            _buffer->buffer_data(audioData, frameCount, channels, sampleRate);
            free(audioData);
            return true;
        }
    }
    return false;
}

void Sound::start(bool looped)
{
    if (state() != AudioState::Playing) {
        if (_buffer->size() > 0) {
            stop();
            auto s { source() };
            s->buffer(_buffer->ID);
            s->looping(looped);
            s->play();
        }
    }
}

void Sound::stop()
{
    if (state() != AudioState::Stopped) {
        source()->stop();
    }
}

auto Sound::duration() const -> f32
{
    i32 size { _buffer->size() };
    i32 channels { _buffer->channels() };
    i32 bits { _buffer->bits() };

    f32 lengthInSamples = { size * 8.0f / (channels * bits) };
    i32 frequency { _buffer->frequency() };

    return lengthInSamples / frequency * 1000;
}

auto Sound::playback_position() const -> f32
{
    if (_buffer->size() > 0) {
        return source()->sec_offset();
    }
    return 0;
}
}