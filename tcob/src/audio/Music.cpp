// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/audio/Music.hpp"

#include <chrono>
#include <memory>
#include <optional>
#include <thread>
#include <utility>
#include <vector>

#include "tcob/audio/Audio.hpp"
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

auto music::info() const -> std::optional<specification>
{
    return _info->Specs;
}

auto music::duration() const -> milliseconds
{
    if (!_decoder || !_info) { return 0ms; }
    if (!_info->Specs.is_valid()) { return 0ms; }

    return milliseconds {(static_cast<f32>(_info->FrameCount) / static_cast<f32>(_info->Specs.SampleRate)) * 1000.0f};
}

auto music::playback_position() const -> milliseconds
{
    if (!_decoder || !_info) { return 0ms; }
    if (!_info->Specs.is_valid()) { return 0ms; }

    return milliseconds {(static_cast<f32>(_samplesPlayed) / static_cast<f32>(_info->Specs.SampleRate) / static_cast<f32>(_info->Specs.Channels)) * 1000.0f};
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
    if (!_decoder) { return load_status::Error; }

    _info = _decoder->open(std::move(in), DecoderContext);
    if (!_info) { return load_status::Error; }
    if (!_info->Specs.is_valid()) { return load_status::Error; }

    create_output(_info->Specs);
    return load_status::Ok;
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
    _decoder->seek_from_start(0ms);

    fill_buffers();

    for (;;) {
        if (_stopRequested) { break; }

        fill_buffers();

        std::this_thread::sleep_for(1ms);

        if (queued_bytes() == 0) { _stopRequested = true; }
    }

    _isRunning     = false;
    _stopRequested = false;
}

void music::stop_stream()
{
    _samplesPlayed = 0;

    _buffers = {};

    if (!_isRunning) { return; }

    _stopRequested = true;
    while (_isRunning) { std::this_thread::yield(); }
    _stopRequested = false;
}

constexpr i64 STREAM_BUFFER_SIZE {8192};
constexpr u8  STREAM_BUFFER_COUNT {4};
constexpr i64 STREAM_BUFFER_THRESHOLD {STREAM_BUFFER_SIZE * (STREAM_BUFFER_COUNT - 1)};

void music::fill_buffers()
{
    while (_buffers.size() < STREAM_BUFFER_COUNT) {
        if (auto const data {_decoder->decode(STREAM_BUFFER_SIZE)}) {
            buffer::information const info {
                .Specs      = _info->Specs,
                .FrameCount = std::ssize(*data) / info.Specs.Channels};
            _buffers.push(buffer::Create(info, *data)); // TODO: reuse buffers
            _samplesPlayed += data->size();
        } else {
            flush_output();
            break;
        }
    }

    if (_buffers.empty()) { return; }

    while (queued_bytes() / sizeof(f32) < STREAM_BUFFER_THRESHOLD) {
        auto& buffer {_buffers.front()};
        write_to_output(buffer.data());
        _buffers.pop();
    }
}
}
