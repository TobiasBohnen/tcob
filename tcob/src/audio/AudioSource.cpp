// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include <utility>

#include "tcob/audio/AudioSource.hpp"

#include "ALObjects.hpp"

namespace tcob::audio {

source::source()
    : Volume {{[&]() { return _source->get_gain(); },
               [&](auto const& value) { _source->set_gain(value); }}}
    , _source {std::make_shared<audio::al::al_source>()}
{
    Volume(1.0f);
}

source::~source() = default;

source::source(source const& other) noexcept
    : source {}
{
    *this = other;
}

auto source::operator=(source const& other) noexcept -> source&
{
    if (this != &other) {
        _source = other._source;
        Volume  = other.Volume();
    }

    return *this;
}

auto source::get_status() const -> playback_status
{
    return _source->get_status();
}

auto source::is_looping() const -> bool
{
    return _source->is_looping();
}

void source::play(bool looping)
{
    if (get_status() == playback_status::Stopped) { // start if stopped
        if (on_start()) {
            _source->set_looping(looping);
        }
    } else if (get_status() == playback_status::Paused) { // resume if paused
        _source->set_looping(looping);
        resume();
    }
}

void source::stop()
{
    if (get_status() != playback_status::Stopped) { // stop if running or paused
        on_stop();
    }
}

void source::restart()
{
    stop();
    play(is_looping());
}

void source::pause()
{
    if (get_status() == playback_status::Running) {
        _source->pause();
    }
}

void source::resume()
{
    if (get_status() == playback_status::Paused) {
        _source->play();
    }
}

void source::toggle_pause()
{
    get_status() == playback_status::Paused ? resume() : pause();
}

auto source::get_source() const -> audio::al::al_source*
{
    return _source.get();
}

////////////////////////////////////////////////////////////

using namespace std::chrono_literals;

decoder::decoder()  = default;
decoder::~decoder() = default;

auto decoder::open(std::shared_ptr<istream> in, std::any& ctx) -> std::optional<buffer::info>
{
    _stream = std::move(in);
    _ctx    = ctx;
    _info   = open();
    return _info;
}

auto decoder::decode_to_buffer(al::al_buffer* buffer, i64 wantSamples) -> bool
{
    if (!_info || wantSamples <= 0) { return false; }

    std::vector<f32> data(static_cast<usize>(wantSamples));
    if (decode(data) > 0) {
        buffer->buffer_data(data, _info->Channels, _info->SampleRate);
        return true;
    }
    return false;
}

auto decoder::decode_to_buffer(std::span<f32> buffer) -> bool
{
    decode(buffer);
    return true; // TODO check
}

auto decoder::get_stream() -> istream&
{
    return *_stream;
}

auto decoder::get_context() -> std::any&
{
    return _ctx;
}

}
