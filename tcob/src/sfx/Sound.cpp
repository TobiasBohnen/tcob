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

#include "AudioIO.hpp"
#include <tcob/core/io/FileStream.hpp>

namespace tcob {

Sound::Sound()
{
    _source = std::make_unique<al::Source>();
    _buffer = std::make_shared<al::Buffer>();
}

Sound::~Sound()
{
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
    //TODO: copy source settings
    _buffer = other._buffer;
    return *this;
}

auto Sound::load(const std::string& filename) -> bool
{
    if (FileSystem::is_file(filename)) {
        InputFileStream stream { filename };
        std::string ext { FileSystem::extension(filename) };

        if (ext == ".wav") {
            u64 frameCount { 0 };
            u32 channels { 0 }, sampleRate { 0 };
            auto audioData { drwav_open_and_read_pcm_frames_s16(&detail::read, &detail::seek_wav, &stream, &channels, &sampleRate, &frameCount, nullptr) };
            if (audioData) {
                _buffer->buffer_data(channels, audioData, static_cast<i32>(frameCount * 2), sampleRate);
                drwav_free(audioData, nullptr);
                return true;
            }
        } else if (ext == ".flac") {
            u64 frameCount { 0 };
            u32 channels { 0 }, sampleRate { 0 };
            auto audioData { drflac_open_and_read_pcm_frames_s16(&detail::read, &detail::seek_flac, &stream, &channels, &sampleRate, &frameCount, nullptr) };
            if (audioData) {
                _buffer->buffer_data(channels, audioData, static_cast<i32>(frameCount * 2), sampleRate);
                drflac_free(audioData, nullptr);
                return true;
            }
        } else if (ext == ".mp3") {
            u64 frameCount { 0 };
            drmp3_config config {};
            auto audioData { drmp3_open_and_read_pcm_frames_s16(&detail::read, &detail::seek_mp3, &stream, &config, &frameCount, nullptr) };
            if (audioData) {
                _buffer->buffer_data(config.channels, audioData, static_cast<i32>(frameCount * 2), config.sampleRate);
                drmp3_free(audioData, nullptr);
                return true;
            }
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
}