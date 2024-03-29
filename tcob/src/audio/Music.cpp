// Copyright (c) 2023 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/audio/Music.hpp"

#include <cassert>
#include <thread>
#include <utility>

#include "tcob/audio/AudioSource.hpp"
#include "tcob/core/ServiceLocator.hpp"
#include "tcob/core/io/FileStream.hpp"
#include "tcob/core/io/FileSystem.hpp"

#include "ALObjects.hpp"

namespace tcob::audio {
using namespace std::chrono_literals;

music::~music()
{
    stop_stream();
}

auto music::get_info() const -> std::optional<buffer::info>
{
    return _info;
}

auto music::open(path const& file) -> load_status
{
    if (!io::is_file(file)) { return load_status::FileNotFound; }

    return open(std::make_shared<io::ifstream>(file), io::get_extension(file));
}

auto music::open(std::shared_ptr<istream> in, string const& ext) -> load_status
{
    stop();

    _decoder = locate_service<decoder::factory>().create_from_sig_or_ext(*in, ext);
    if (_decoder) {
        _info = _decoder->open(std::move(in), DecoderContext);
        return _info ? load_status::Ok : load_status::Error;
    }

    return load_status::Error;
}

auto music::get_duration() const -> milliseconds
{
    if (!_decoder || !_info) {
        return 0ms;
    }

    if (_info->SampleRate == 0) {
        return 0ms;
    }

    return milliseconds {(static_cast<f32>(_info->FrameCount) / static_cast<f32>(_info->SampleRate)) * 1000.0f};
}

auto music::get_playback_position() const -> milliseconds
{
    if (!_decoder || !_info) {
        return 0ms;
    }

    if (_info->Channels == 0 || _info->SampleRate == 0) {
        return 0ms;
    }

    return milliseconds {(static_cast<f32>(_samplesPlayed) / static_cast<f32>(_info->SampleRate) / static_cast<f32>(_info->Channels)) * 1000.0f};
}

void music::on_start()
{
    stop_stream();
    _thread = std::jthread {&music::update_stream, this};
}

void music::on_stop()
{
    stop_stream();
}

auto music::can_start() const -> bool
{
    return _decoder != nullptr;
}

void music::update_stream()
{
    auto stoken {_thread.get_stop_token()};

    initialize_buffers();

    auto* s {get_source()};
    s->play();

    for (;;) {
        if (stoken.stop_requested() || get_status() == source::status::Stopped) {
            s->stop();
            s->set_buffer(0);
            break;
        }

        if (get_status() == source::status::Playing) {
            i32 const  processedCount {s->get_buffers_processed()};
            auto const bufferIDs {s->unqueue_buffers(processedCount)};
            assert(processedCount == std::ssize(bufferIDs));
            if (processedCount > 0) {
                for (u32 bufferID : bufferIDs) {
                    _samplesPlayed += static_cast<i32>(al::al_buffer::GetSize(bufferID) / sizeof(f32)); // buffer is float32
                }

                queue_buffers(bufferIDs);
            }
        }

        std::this_thread::sleep_for(1ms);
    }
}

void music::stop_stream()
{
    _thread.request_stop();
}

void music::queue_buffers(std::vector<u32> const& bufferIDs)
{
    auto* s {get_source()};

    for (u32 bufferID : bufferIDs) {
        for (u32 i {0}; i < STREAM_BUFFER_COUNT; ++i) {
            if (bufferID == _buffers[i]->get_id()) {
                if (_decoder->decode_to_buffer(_buffers[i].get(), STREAM_BUFFER_SIZE)) {
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
