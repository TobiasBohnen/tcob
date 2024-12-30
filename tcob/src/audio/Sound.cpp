// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/audio/Sound.hpp"

#include "tcob/audio/Buffer.hpp"
#include "tcob/core/Common.hpp"
#include "tcob/core/ServiceLocator.hpp"
#include "tcob/core/TaskManager.hpp"
#include "tcob/core/io/FileStream.hpp"
#include "tcob/core/io/FileSystem.hpp"

#include <utility>

#include "ALObjects.hpp"

namespace tcob::audio {
using namespace std::chrono_literals;

sound::sound()
    : _buffer {std::make_unique<audio::al::al_buffer>()}
{
}

sound::sound(audio::buffer const& buffer)
    : _buffer {std::make_unique<audio::al::al_buffer>()}
{
    auto const& info {buffer.info()};
    _buffer->buffer_data(buffer.data(), info.Channels, info.SampleRate);
}

sound::~sound()
{
    stop_source();
}

auto sound::info() const -> std::optional<buffer::information>
{
    return _info;
}

auto sound::load(path const& file) noexcept -> load_status
{
    return load(std::make_shared<io::ifstream>(file), io::get_extension(file));
}

auto sound::load(std::shared_ptr<io::istream> in, string const& ext) noexcept -> load_status
{
    if (!in || !(*in)) { return load_status::Error; }

    stop_source();

    buffer bfr;
    if (bfr.load(std::move(in), ext, DecoderContext) == load_status::Ok) {
        _info = bfr.info();
        _buffer->buffer_data(bfr.data(), _info.Channels, _info.SampleRate);
        return load_status::Ok;
    }

    return load_status::Error;
}

auto sound::load_async(path const& file) noexcept -> std::future<load_status>
{
    return locate_service<task_manager>().run_async<load_status>([&, file]() { return load(file); });
}

auto sound::on_start() -> bool
{
    if (_buffer->get_size() <= 0) { return false; }

    stop();
    auto* s {get_impl()};
    s->set_buffer(_buffer->get_id());
    s->play();

    return true;
}

auto sound::on_stop() -> bool
{
    stop_source();
    return true;
}

auto sound::duration() const -> milliseconds
{
    i32 const size {_buffer->get_size()};
    i32 const channels {_buffer->get_channels()};
    i32 const bits {_buffer->get_bits()};
    i32 const frequency {_buffer->get_frequency()};

    f32 const lengthInSamples {size / (channels * (bits / 8.0f))};

    return milliseconds {(lengthInSamples / frequency) * 1000};
}

auto sound::playback_position() const -> milliseconds
{
    return _buffer->get_size() > 0
        ? milliseconds {get_impl()->get_sec_offset()}
        : 0ms;
}

void sound::stop_source()
{
    auto* s {get_impl()};
    s->stop();
    s->set_buffer(0);
}
}
