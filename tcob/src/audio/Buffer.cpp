// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/audio/Buffer.hpp"

#include <any>
#include <chrono>
#include <future>
#include <memory>
#include <optional>
#include <span>
#include <thread>
#include <utility>
#include <vector>

#include "tcob/audio/Audio.hpp"
#include "tcob/core/ServiceLocator.hpp"
#include "tcob/core/TaskManager.hpp"
#include "tcob/core/io/FileStream.hpp"
#include "tcob/core/io/FileSystem.hpp"
#include "tcob/core/io/Stream.hpp"

namespace tcob::audio {
using namespace std::chrono_literals;

auto buffer::info() const -> information const&
{
    return _info;
}

auto buffer::Create(specification const& info, std::span<f32 const> data) -> buffer
{
    buffer retValue;
    retValue._info.Specs      = info;
    retValue._info.FrameCount = std::ssize(data) / info.Channels;
    retValue._buffer          = {data.begin(), data.end()};
    return retValue;
}

auto buffer::Load(path const& file) -> std::optional<buffer>
{
    buffer     retValue;
    auto const err {retValue.load(file, {})};
    if (err) { return retValue; }
    return std::nullopt;
}

auto buffer::Load(std::shared_ptr<io::istream> in, string const& ext) -> std::optional<buffer>
{
    buffer     retValue;
    auto const err {retValue.load(std::move(in), ext, {})};
    if (err) { return retValue; }
    return std::nullopt;
}

auto buffer::load(path const& file, std::any const& ctx) noexcept -> bool
{
    return load(std::make_shared<io::ifstream>(file), io::get_extension(file), ctx);
}

auto buffer::load(std::shared_ptr<io::istream> in, string const& ext, std::any const& ctx) noexcept -> bool
{
    _buffer.clear();
    if (!in || !(*in)) { return false; }

    auto decoder {locate_service<decoder::factory>().create_from_magic(*in, ext)};
    if (!decoder) { return false; }

    if (auto info {decoder->open(std::move(in), ctx)}) {
        _info = *info;
        decoder->seek_from_start(0ms);

        std::vector<f32> buffer(static_cast<usize>(_info.Specs.Channels * _info.FrameCount));
        auto const       size {decoder->decode(buffer)};
        if (size > 0) {
            buffer.resize(size);
            _buffer = std::move(buffer);
            return true;
        }
    }

    return false;
}

auto buffer::load_async(path const& file, std::any& ctx) noexcept -> std::future<bool>
{
    return locate_service<task_manager>().run_async<bool>([&, file, ctx]() mutable { return load(file, ctx); });
}

auto buffer::save(path const& file) const noexcept -> bool
{
    io::ofstream of {file};
    return save(of, io::get_extension(file));
}

auto buffer::save(io::ostream& out, string const& ext) const noexcept -> bool
{
    if (_info.FrameCount == 0) { return false; }

    if (auto enc {locate_service<encoder::factory>().create(ext)}) {
        return enc->encode(_buffer, _info, out);
    }

    return false;
}

auto buffer::save_async(path const& file) const noexcept -> std::future<bool>
{
    std::promise<bool> pro;
    auto               retValue {pro.get_future()};
    std::thread([*this, file](std::promise<bool> p) { p.set_value(save(file)); }, std::move(pro)).detach();
    return retValue;
}

////////////////////////////////////////////////////////////

using namespace std::chrono_literals;

decoder::decoder()  = default;
decoder::~decoder() = default;

auto decoder::open(std::shared_ptr<io::istream> in, std::any const& ctx) -> std::optional<buffer::information>
{
    _stream = std::move(in);
    _ctx    = ctx;
    _info   = open();
    return _info;
}

auto decoder::stream() -> io::istream&
{
    return *_stream;
}

auto decoder::context() -> std::any&
{
    return _ctx;
}

}
