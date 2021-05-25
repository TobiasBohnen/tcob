// Copyright (c) 2021 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include <tcob/sfx/AudioBuffer.hpp>

#include <AL/al.h>
#define DR_FLAC_NO_STDIO
#include <dr_libs/dr_flac.h>
#define DR_MP3_NO_STDIO
#include <dr_libs/dr_mp3.h>
#define DR_WAV_NO_STDIO
#include <dr_libs/dr_wav.h>

#include <tcob/core/io/FileStream.hpp>

namespace tcob {

auto read(void* userdata, void* buffer, isize bytesToRead) -> isize
{
    InputFileStream* stream { reinterpret_cast<InputFileStream*>(userdata) };
    return stream->read(reinterpret_cast<byte*>(buffer), bytesToRead);
}

auto seek_wav(void* userdata, i32 offset, drwav_seek_origin origin) -> u32
{
    InputFileStream* stream { reinterpret_cast<InputFileStream*>(userdata) };
    auto dir { origin == drwav_seek_origin_current ? std::ios_base::cur : std::ios_base::beg };
    return static_cast<u32>(stream->seek(offset, dir));
}

auto seek_flac(void* userdata, i32 offset, drflac_seek_origin origin) -> u32
{
    InputFileStream* stream { reinterpret_cast<InputFileStream*>(userdata) };
    auto dir { origin == drflac_seek_origin_current ? std::ios_base::cur : std::ios_base::beg };
    return static_cast<u32>(stream->seek(offset, dir));
}

auto seek_mp3(void* userdata, i32 offset, drmp3_seek_origin origin) -> u32
{
    InputFileStream* stream { reinterpret_cast<InputFileStream*>(userdata) };
    auto dir { origin == drmp3_seek_origin_current ? std::ios_base::cur : std::ios_base::beg };
    return static_cast<u32>(stream->seek(offset, dir));
}

AudioBuffer::AudioBuffer()
{
    _source = std::make_unique<al::Source>();
    _buffer = std::make_shared<al::Buffer>();
}

AudioBuffer::~AudioBuffer()
{
    _source->buffer(0);
    _buffer = nullptr;
    _source = nullptr;
}

auto AudioBuffer::load(const std::string& filename) -> bool
{
    if (FileSystem::is_file(filename)) {
        InputFileStream stream { filename };
        std::string ext { FileSystem::extension(filename) };

        if (ext == ".wav") {
            u64 frameCount { 0 };
            u32 channels { 0 }, sampleRate { 0 };
            auto audioData { drwav_open_and_read_pcm_frames_s16(&read, &seek_wav, &stream, &channels, &sampleRate, &frameCount, nullptr) };
            if (audioData) {
                _buffer->buffer_data(channels, audioData, static_cast<i32>(frameCount * 2), sampleRate);
                drwav_free(audioData, nullptr);
                return true;
            }
        } else if (ext == ".flac") {
            u64 frameCount { 0 };
            u32 channels { 0 }, sampleRate { 0 };
            auto audioData { drflac_open_and_read_pcm_frames_s16(&read, &seek_flac, &stream, &channels, &sampleRate, &frameCount, nullptr) };
            if (audioData) {
                _buffer->buffer_data(channels, audioData, static_cast<i32>(frameCount * 2), sampleRate);
                drflac_free(audioData, nullptr);
                return true;
            }
        } else if (ext == ".mp3") {
            u64 frameCount { 0 };
            drmp3_config config {};
            auto audioData { drmp3_open_and_read_pcm_frames_s16(&read, &seek_mp3, &stream, &config, &frameCount, nullptr) };
            if (audioData) {
                _buffer->buffer_data(config.channels, audioData, static_cast<i32>(frameCount * 2), config.sampleRate);
                drmp3_free(audioData, nullptr);
                return true;
            }
        }
    }
    return false;
}

void AudioBuffer::play()
{
    stop();
    _source->buffer(_buffer->ID);
    _source->play();
}

void AudioBuffer::stop()
{
    _source->stop();
}
}