// Copyright (c) 2021 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include <tcob/sfx/Music.hpp>

#include <AL/al.h>

#include <tcob/core/io/FileStream.hpp>

namespace tcob {

detail::AudioDecoder::AudioDecoder(const std::string& filename)
    : _stream { std::make_unique<InputFileStream>(filename) }
{
}

auto detail::AudioDecoder::buffer_data(u32 buffer) -> bool
{

    return true;
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
        const std::lock_guard lock { _mutex };

        _file = filename;
        _decoder = nullptr;
        std::string ext { FileSystem::extension(filename) };

        if (ext == ".wav") {

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

    const std::lock_guard lock { _mutex };

    if (_source->state() != AudioState::Playing) {
        stop();

        std::vector<u32> buffers {};
        for (u32 i { 0 }; i < MUSIC_BUFFER_COUNT; ++i) {
            buffers.push_back(_buffers[i].ID);
        }

        queue_buffers(buffers);
        _source->play();
        _thread = std::thread { &Music::update_stream, this };
    }
}

void Music::stop()
{
    const std::lock_guard lock { _mutex };

    if (_source->state() != AudioState::Stopped) {
        _source->stop();
        if (_thread.joinable())
            _thread.join();
    }
}

using namespace std::chrono_literals;
void Music::update_stream()
{
    while (_source->state() != AudioState::Stopped) {

        bool paused { false };
        {
            const std::lock_guard lock { _mutex };
            paused = _source->state() == AudioState::Paused;
        }

        if (!paused) {
            i32 processed { _source->buffers_processed() };
            if (processed > 0) {
                queue_buffers(_source->unqueue_buffers(processed));
            }
        }
        std::this_thread::sleep_for(1ms);
    }
}

void Music::queue_buffers(const std::vector<u32>& buffers)
{
    const std::lock_guard lock { _mutex };
    for (u32 buffer : buffers) {
        if (_decoder->buffer_data(buffer)) {
            alSourceQueueBuffers(_source->ID, 1, &buffer);
        } else {
            return;
        }
    }
}

}