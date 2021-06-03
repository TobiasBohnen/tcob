// Copyright (c) 2021 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include <tcob/sfx/Music.hpp>

#include <AL/al.h>
#include <cassert>

#include "AudioCodecs.hpp"
#include <tcob/core/io/FileStream.hpp>

namespace tcob {

Music::Music()
{
}

Music::~Music()
{
    stop_stream();
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
            _decoder = std::make_unique<detail::FlacDecoder>(filename);
        } else if (ext == ".mp3") {
            _decoder = std::make_unique<detail::Mp3Decoder>(filename);
        } else if (ext == ".ogg") {
            _decoder = std::make_unique<detail::VorbisDecoder>(filename);
        }

        return true;
    }
    return false;
}

void Music::start(bool looped)
{
    if (!_decoder) {
        return;
    }

    if (state() != AudioState::Playing) {
        stop_stream();
        source()->looping(looped);
        _thread = std::thread { &Music::update_stream, this };
    }
}

void Music::stop()
{
    if (state() != AudioState::Stopped) {
        stop_stream();
    }
}

auto Music::duration() const -> f32
{
    return (static_cast<f32>(_decoder->info().SampleCount) / _decoder->info().Frequency) * 1000.f;
}

auto Music::playback_position() const -> f32
{
    return (static_cast<f32>(_samplesPlayed) / _decoder->info().Frequency / _decoder->info().Channels) * 1000.f;
}

using namespace std::chrono_literals;
void Music::update_stream()
{
    auto s { source() };

    fill_buffers();
    s->play();

    for (;;) {
        if (state() == AudioState::Stopped) {
            _requestStop = true;
        }

        if (_requestStop) {
            s->stop();
            s->buffer(0);
            _requestStop = false;
            break;
        }

        if (state() == AudioState::Playing) {
            auto processed { s->buffers_processed() };
            auto buffers { s->unqueue_buffers(processed) };
            assert(processed == buffers.size());
            if (processed > 0) {
                for (auto buffer : buffers) {
                    i32 size;
                    alGetBufferi(buffer, AL_SIZE, &size);
                    _samplesPlayed += (size / sizeof(f32)); //buffer is float32
                }

                queue_buffers(buffers);
            }
        }

        std::this_thread::sleep_for(1ms);
    }
}

void Music::stop_stream()
{
    _requestStop = true;
    if (_thread.joinable())
        _thread.join();
    _requestStop = false;
}

void Music::queue_buffers(const std::vector<u32>& buffers)
{
    auto s { source() };

    for (u32 buffer : buffers) {
        if (_decoder->buffer_data(buffer)) {
            s->queue_buffers(&buffer, 1);
        } else {
            if (s->looping()) {
                while (s->buffers_queued() > 0) {
                }
                s->unqueue_buffers(s->buffers_processed());
                fill_buffers();
            }
            return;
        }
    }
}

void Music::fill_buffers()
{
    _samplesPlayed = 0;
    _decoder->seek(0);
    std::vector<u32> buffers {};
    for (u32 i { 0 }; i < MUSIC_BUFFER_COUNT; ++i) {
        _buffers[i] = std::make_unique<al::Buffer>();
        buffers.push_back(_buffers[i]->ID);
    }
    queue_buffers(buffers);
}
}