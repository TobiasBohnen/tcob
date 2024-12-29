// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/audio/Music.hpp"

#include <cassert>
#include <thread>

#include "tcob/core/ServiceLocator.hpp"
#include "tcob/core/TaskManager.hpp"
#include "tcob/core/io/FileStream.hpp"
#include "tcob/core/io/FileSystem.hpp"

#include "ALObjects.hpp"

namespace tcob::audio {
using namespace std::chrono_literals;

music::~music()
{
    stop_stream();
}

auto music::info() const -> std::optional<buffer::information>
{
    return _info;
}

auto music::open(path const& file) -> load_status
{
    return open(std::make_shared<io::ifstream>(file), io::get_extension(file));
}

auto music::open(std::shared_ptr<io::istream> in, string const& ext) -> load_status
{
    if (!in || !(*in)) { return load_status::Error; }

    stop();

    _decoder = locate_service<decoder::factory>().create_from_sig_or_ext(*in, ext);
    if (_decoder) {
        _info = _decoder->open(std::move(in), DecoderContext);
        return _info ? load_status::Ok : load_status::Error;
    }

    return load_status::Error;
}

auto music::duration() const -> milliseconds
{
    if (!_decoder || !_info) { return 0ms; }
    if (_info->SampleRate == 0) { return 0ms; }

    return milliseconds {(static_cast<f32>(_info->FrameCount) / static_cast<f32>(_info->SampleRate)) * 1000.0f};
}

auto music::playback_position() const -> milliseconds
{
    if (!_decoder || !_info) { return 0ms; }
    if (_info->Channels == 0 || _info->SampleRate == 0) { return 0ms; }

    return milliseconds {(static_cast<f32>(_samplesPlayed) / static_cast<f32>(_info->SampleRate) / static_cast<f32>(_info->Channels)) * 1000.0f};
}

auto music::on_start() -> bool
{
    if (_decoder == nullptr) { return false; }

    stop_stream();
    _isRunning = true;
    locate_service<task_manager>().run_async<void>([this]() { update_stream(); });

    return true;
}

auto music::on_stop() -> bool
{
    stop_stream();
    return true;
}

void music::update_stream()
{
    initialize_buffers();

    auto* s {get_source()};
    s->play();

    for (;;) {
        if (_stopRequested || status() == playback_status::Stopped) {
            s->stop();
            s->set_buffer(0);
            break;
        }

        if (status() == playback_status::Running) {
            i32 const  processedCount {s->get_buffers_processed()};
            auto const bufferIDs {s->unqueue_buffers(processedCount)};
            assert(processedCount == std::ssize(bufferIDs));
            if (processedCount > 0) {
                for (u32 bufferID : bufferIDs) {
                    _samplesPlayed += static_cast<i32>(static_cast<usize>(al::al_buffer::GetSize(bufferID)) / sizeof(f32)); // buffer is float32
                }

                queue_buffers(bufferIDs);
            }
        }

        std::this_thread::sleep_for(1ms);
    }

    _isRunning = false;
}

void music::stop_stream()
{
    if (!_isRunning) { return; }

    _stopRequested = true;
    while (_isRunning) { std::this_thread::yield(); }
    _stopRequested = false;
}

void music::queue_buffers(std::vector<u32> const& bufferIDs)
{
    auto* s {get_source()};

    for (u32 bufferID : bufferIDs) {
        for (u32 i {0}; i < STREAM_BUFFER_COUNT; ++i) {
            if (bufferID == _buffers[i]->get_id()) {
                if (auto const data {_decoder->decode(STREAM_BUFFER_SIZE)}) {
                    _buffers[i]->buffer_data(*data, _info->Channels, _info->SampleRate);
                    s->queue_buffers(&bufferID, 1);
                } else {
                    if (s->is_looping()) {
                        while (s->get_buffers_queued() > 0) { }
                        s->unqueue_buffers(s->get_buffers_processed());
                        initialize_buffers();
                    }
                    return;
                }

                break;
            }
        }
    }
}

void music::initialize_buffers()
{
    _samplesPlayed = 0;
    _decoder->seek_from_start(0ms);
    std::vector<u32> bufferIDs {};
    for (u8 i {0}; i < STREAM_BUFFER_COUNT; ++i) {
        _buffers[i] = std::make_shared<audio::al::al_buffer>();
        bufferIDs.push_back(_buffers[i]->get_id());
    }
    queue_buffers(bufferIDs);
}
}
