// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/audio/Music.hpp"

#include <cassert>
#include <chrono>
#include <memory>
#include <optional>
#include <thread>
#include <utility>
#include <vector>

#include "tcob/audio/AudioSystem.hpp"
#include "tcob/audio/Buffer.hpp"
#include "tcob/core/Common.hpp"
#include "tcob/core/ServiceLocator.hpp"
#include "tcob/core/TaskManager.hpp"
#include "tcob/core/io/FileStream.hpp"
#include "tcob/core/io/FileSystem.hpp"
#include "tcob/core/io/Stream.hpp"

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
        if (_info) {
            create_output(*_info);
            return load_status::Ok;
        }

        return load_status::Error;
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
    _samplesPlayed = 0;
    _decoder->seek_from_start(0ms);

    auto& out {get_output()};
    fill_buffers(out);

    for (;;) {
        if (_stopRequested) {
            out.clear();
            out.unbind();
            break;
        }

        std::this_thread::sleep_for(1ms);

        if (out.available_bytes() == 0) {
            _stopRequested = true;
        }

        fill_buffers(out);
    }

    _isRunning     = false;
    _stopRequested = false;
}

void music::stop_stream()
{
    if (!_isRunning) { return; }

    _stopRequested = true;
    while (_isRunning) { std::this_thread::yield(); }
    _stopRequested = false;
}

void music::fill_buffers(audio::output& out)
{
    bool flush {false};
    while (_buffers.size() < STREAM_BUFFER_COUNT) {
        if (auto const data {_decoder->decode(STREAM_BUFFER_SIZE)}) {
            buffer::information info;
            info.Channels   = _info->Channels;
            info.SampleRate = _info->SampleRate;
            info.FrameCount = data->size() / info.Channels;
            _buffers.push(buffer::Create(info, *data));
            _samplesPlayed += data->size();
        } else {
            flush = true;
            break;
        }
    }

    while (out.queued_bytes() / sizeof(f32) < STREAM_BUFFER_SIZE * (STREAM_BUFFER_COUNT - 1) && !_buffers.empty()) {
        auto const& buffer {_buffers.front()};
        out.put(buffer.data());
        _buffers.pop();
    }

    if (flush) {
        out.flush();
    }
}
}
