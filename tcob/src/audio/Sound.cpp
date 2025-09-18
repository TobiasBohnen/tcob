// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/audio/Sound.hpp"

#include <future>
#include <memory>
#include <optional>
#include <utility>

#include "tcob/audio/Audio.hpp"
#include "tcob/audio/Buffer.hpp"
#include "tcob/core/ServiceLocator.hpp"
#include "tcob/core/TaskManager.hpp"
#include "tcob/core/io/FileStream.hpp"
#include "tcob/core/io/FileSystem.hpp"
#include "tcob/core/io/Stream.hpp"

namespace tcob::audio {
using namespace std::chrono_literals;

sound::sound(buffer buffer)
    : _buffer {std::move(buffer)}
{
    create_output();
}

auto sound::info() const -> std::optional<specification>
{
    return _buffer.info().Specs;
}

auto sound::duration() const -> milliseconds
{
    auto const& info {_buffer.info()};
    return milliseconds {(static_cast<f32>(info.FrameCount) / static_cast<f32>(info.Specs.SampleRate)) * 1000};
}

auto sound::load(path const& file) noexcept -> bool
{
    return load(std::make_shared<io::ifstream>(file), io::get_extension(file));
}

auto sound::load(std::shared_ptr<io::istream> in, string const& ext) noexcept -> bool
{
    if (!in || !(*in)) { return false; }

    stop();

    if (!_buffer.load(std::move(in), ext, DecoderContext)) { return false; }
    if (!_buffer.info().Specs.is_valid()) { return false; }

    create_output();
    return true;
}

auto sound::load_async(path const& file) noexcept -> std::future<bool>
{
    return locate_service<task_manager>().run_async<bool>([&, file] { return load(file); });
}

auto sound::on_start() -> bool
{
    if (_buffer.data().empty()) { return false; }

    write_to_output(_buffer.data());
    flush_output();

    return true;
}

auto sound::on_stop() -> bool
{
    return true;
}

}
