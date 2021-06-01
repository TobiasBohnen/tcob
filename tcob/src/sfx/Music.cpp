// Copyright (c) 2021 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include <tcob/sfx/Music.hpp>

#include <AL/al.h>

#include "AudioCodecs.hpp"
#include <tcob/core/io/FileStream.hpp>

namespace tcob {

detail::AudioDecoder::AudioDecoder(const std::string& filename)
    : _stream { std::make_unique<InputFileStream>(filename) }
{
}

auto detail::AudioDecoder::buffer_data(u32 buffer) -> bool
{
    std::vector<i16> data {};
    data.reserve(MUSIC_BUFFER_SIZE);
    i32 frameCount { 0 };
    bool ok { read_data(data.data(), frameCount) };

    auto audioInfo { info() };
    i32 framesize { static_cast<i32>(frameCount * audioInfo.Channels * sizeof(i16)) }; //assumes short frames
    alBufferData(buffer,
        audioInfo.Channels == 1 ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16,
        data.data(), framesize, audioInfo.Frequency);

    return ok;
}

auto detail::AudioDecoder::stream() const -> InputFileStream*
{
    return _stream.get();
}

////////////////////////////////////////////////////////////

Music::Music()
    : _source { std::make_unique<al::Source>() }
{
}

Music::~Music()
{
    stop();
    _source = nullptr;
}

auto Music::open(const std::string& filename) -> bool
{
    if (FileSystem::is_file(filename)) {
        stop();

        _file = filename;
        _decoder = nullptr;
        std::string ext { FileSystem::extension(filename) };

        if (ext == ".wav") {
            _decoder = std::make_unique<detail::WavDecoder>(filename);
        } else if (ext == ".flac") {

        } else if (ext == ".mp3") {

        } else if (ext == ".ogg") {
        }

        return true;
    }
    return false;
}

void Music::play()
{
    if (!_decoder) {
        return;
    }

    if (_source->state() != AudioState::Playing) {
        _decoder->seek(0);
        _requestStop = false;
        _thread = std::thread { &Music::update_stream, this };
    }
}

void Music::stop()
{
    if (_source->state() != AudioState::Stopped) {
        _requestStop = true;
        if (_thread.joinable())
            _thread.join();
    }
}

using namespace std::chrono_literals;
void Music::update_stream()
{
    std::vector<u32> buffers {};
    for (u32 i { 0 }; i < MUSIC_BUFFER_COUNT; ++i) {
        _buffers[i] = std::make_unique<al::Buffer>();
        buffers.push_back(_buffers[i]->ID);
    }
    queue_buffers(buffers);
    _source->play();

    for (;;) {
        if (_source->state() != AudioState::Paused) {
            i32 processed { _source->buffers_processed() };
            if (processed > 0) {
                queue_buffers(_source->unqueue_buffers(processed));
            }
        }

        if (_requestStop) {
            _source->stop();
            _source->unqueue_buffers(_source->buffers_queued());
            _source->buffer(0);
            _requestStop = false;
            break;
        }

        std::this_thread::sleep_for(1ms);
    }
}

void Music::queue_buffers(const std::vector<u32>& buffers)
{
    for (u32 buffer : buffers) {
        if (_decoder->buffer_data(buffer)) {
            alSourceQueueBuffers(_source->ID, 1, &buffer);
        } else {
            return;
        }
    }
}

}