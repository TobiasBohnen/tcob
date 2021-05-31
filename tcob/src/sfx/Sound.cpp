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

#include "AudioIO.hpp"
#include <tcob/core/io/FileStream.hpp>

namespace tcob {

Sound::Sound()
    : _source { std::make_unique<al::Source>() }
    , _buffer { std::make_shared<al::Buffer>() }
{
}

Sound::~Sound()
{
    stop();
    _source->buffer(0);
    _buffer = nullptr;
    _source = nullptr;
}

Sound::Sound(const Sound& other)
{
    *this = other;
}

auto Sound::operator=(const Sound& other) -> Sound&
{
    _source = std::make_unique<al::Source>();
    _source->pitch(other._source->pitch());
    _source->gain(other._source->gain());
    _source->position(other._source->position());
    _source->direction(other._source->direction());
    _source->rolloff_factor(other._source->rolloff_factor());
    _source->source_relatvie(other._source->source_relatvie());

    _buffer = other._buffer;
    return *this;
}

auto Sound::load(const std::string& filename) -> bool
{
    if (FileSystem::is_file(filename)) {
        InputFileStream stream { filename };
        std::string ext { FileSystem::extension(filename) };

        i16* audioData { nullptr };
        u64 frameCount { 0 };
        u32 channels { 0 }, sampleRate { 0 };

        if (ext == ".wav") {
            audioData = drwav_open_and_read_pcm_frames_s16(&detail::read, &detail::seek_wav, &stream, &channels, &sampleRate, &frameCount, nullptr);
        } else if (ext == ".flac") {
            audioData = drflac_open_and_read_pcm_frames_s16(&detail::read, &detail::seek_flac, &stream, &channels, &sampleRate, &frameCount, nullptr);
        } else if (ext == ".mp3") {
            drmp3_config config {};
            audioData = drmp3_open_and_read_pcm_frames_s16(&detail::read, &detail::seek_mp3, &stream, &config, &frameCount, nullptr);
            channels = config.channels;
            sampleRate = config.sampleRate;
        } else if (ext == ".ogg") {
            auto buffer { stream.read_all() };
            i32 c, sr;
            frameCount = stb_vorbis_decode_memory(reinterpret_cast<u8*>(buffer.data()), static_cast<i32>(buffer.size()), &c, &sr, &audioData);
            channels = static_cast<u32>(c);
            sampleRate = static_cast<u32>(sr);
        }

        if (audioData) {
            _buffer->buffer_data(channels, audioData, frameCount, sampleRate);
            free(audioData);
            return true;
        }
    }
    return false;
}

void Sound::play()
{
    if (_buffer->size() > 0) {
        stop();
        _source->buffer(_buffer->ID);
        _source->play();
    }
}

void Sound::stop()
{
    _source->stop();
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
        return _source->sec_offset();
    }
    return 0;
}
}