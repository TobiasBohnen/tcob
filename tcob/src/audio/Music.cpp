// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/audio/Music.hpp"

#include <algorithm>
#include <cassert>
#include <chrono>
#include <memory>
#include <optional>
#include <thread>
#include <utility>

#include "tcob/audio/Audio.hpp"
#include "tcob/audio/Buffer.hpp"
#include "tcob/core/Common.hpp"
#include "tcob/core/ServiceLocator.hpp"
#include "tcob/core/TaskManager.hpp"
#include "tcob/core/easing/Tween.hpp"
#include "tcob/core/io/FileStream.hpp"
#include "tcob/core/io/FileSystem.hpp"
#include "tcob/core/io/Stream.hpp"

namespace tcob::audio {
using namespace std::chrono_literals;

music::~music()
{
    stop_stream();
    locate_service<task_manager>().drop_deferred(_deferred);
}

auto music::info() const -> std::optional<specification>
{
    return _info;
}

auto music::duration() const -> milliseconds
{
    if (!_decoder || !_info) { return 0ms; }
    if (!_info->is_valid()) { return 0ms; }

    return milliseconds {(static_cast<f32>(_totalFrameCount) / static_cast<f32>(_info->SampleRate)) * 1000.0f};
}

auto music::playback_position() const -> milliseconds
{
    if (!_decoder || !_info) { return 0ms; }
    if (!_info->is_valid()) { return 0ms; }

    return milliseconds {(static_cast<f32>(_samplesPlayed) / static_cast<f32>(_info->SampleRate) / static_cast<f32>(_info->Channels)) * 1000.0f};
}

auto music::open(path const& file) -> bool
{
    return open(std::make_shared<io::ifstream>(file), io::get_extension(file));
}

auto music::open(std::shared_ptr<io::istream> in, string const& ext) -> bool
{
    if (!in || !(*in)) { return false; }

    stop();

    _decoder = locate_service<decoder::factory>().create_from_magic(*in, ext);
    if (!_decoder) { return false; }

    auto const info {_decoder->open(std::move(in), DecoderContext)};
    _info            = info->Specs;
    _totalFrameCount = info->FrameCount;

    if (!_info) { return false; }
    if (!_info->is_valid()) { return false; }

    create_output();
    return true;
}

auto music::on_start() -> bool
{
    if (_decoder == nullptr) { return false; }

    stop_stream();
    _isRunning = true;

    auto& tm {locate_service<task_manager>()};
    tm.run_async<void>([this] { update_stream(); });

    if (*FadeIn > 0ms) {
        tm.drop_deferred(_deferred);
        _fadeTween = make_unique_tween<linear_tween<f32>>(*FadeIn, 0.f, *Volume);
        _fadeTween->Value.Changed.connect([this](f32 val) { Volume = val; });
        _fadeTween->Finished.connect([this] { _deferred = INVALID_ID; });
        _fadeTween->start();
        _deferred = tm.run_deferred([this](def_task const& ctx) {
            _fadeTween->update(ctx.DeltaTime);
            ctx.Finished = _fadeTween->state() == playback_state::Stopped;
        });
    }

    return true;
}

auto music::on_stop() -> bool
{
    auto& tm {locate_service<task_manager>()};

    if (*FadeOut > 0ms) {
        tm.drop_deferred(_deferred);
        _fadeTween = make_unique_tween<linear_tween<f32>>(FadeOut, Volume, 0.f);
        _fadeTween->Value.Changed.connect([this](f32 val) { Volume = val; });
        _fadeTween->Finished.connect([this] {
            stop_stream();
            stop_output();
            _deferred = INVALID_ID;
        });
        _fadeTween->start();
        _deferred = tm.run_deferred([this](def_task const& ctx) {
            _fadeTween->update(ctx.DeltaTime);
            ctx.Finished = _fadeTween->state() == playback_state::Stopped;
        });
        return false;
    }

    stop_stream();
    return true;
}

void music::update_stream()
{
    _decoder->seek_from_start(0ms);

    while (!_stopRequested) {
        fill_buffers();

        std::this_thread::sleep_for(1ms);

        if (queued_bytes() == 0) { break; }
    }

    _isRunning     = false;
    _stopRequested = false;
}

void music::stop_stream()
{
    _samplesPlayed = 0;

    _bufferQueue = {};
    for (auto& b : _buffers) { b.Queued = false; }

    if (!_isRunning) { return; }

    _stopRequested = true;
    while (_isRunning) { std::this_thread::yield(); }
    _stopRequested = false;
}

void music::fill_buffers()
{
    while (_bufferQueue.size() < STREAM_BUFFER_COUNT) {
        // find unused buffer
        auto it {std::ranges::find_if(_buffers, [](auto const& b) { return !b.Queued; })};
        assert(it != _buffers.end());
        stream_buffer* buffer {&(*it)};

        // decode to buffer
        buffer->Size = _decoder->decode(buffer->Data);
        if (buffer->Size > 0) {
            buffer->Queued = true;
            _bufferQueue.push(buffer);
            _samplesPlayed += buffer->Size;
        } else {
            flush_output();
            break;
        }
    }

    if (_bufferQueue.empty()) { return; }

    // send data to output
    while ((queued_bytes() / sizeof(f32)) < STREAM_BUFFER_THRESHOLD) {
        auto& buffer {_bufferQueue.front()};
        write_to_output({buffer->Data.data(), static_cast<usize>(buffer->Size)});
        buffer->Queued = false;
        _bufferQueue.pop();
    }
}
}
