// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/audio/Buffer.hpp"

#include <any>
#include <future>
#include <memory>
#include <optional>
#include <span>
#include <thread>
#include <utility>
#include <vector>

#include "tcob/core/Common.hpp"
#include "tcob/core/ServiceLocator.hpp"
#include "tcob/core/TaskManager.hpp"
#include "tcob/core/io/FileStream.hpp"
#include "tcob/core/io/FileSystem.hpp"
#include "tcob/core/io/Stream.hpp"

namespace tcob::audio {

auto buffer::info() const -> information const&
{
    return _info;
}

auto buffer::data() const -> std::span<f32 const>
{
    return _buffer;
}

auto buffer::data() -> std::span<f32>
{
    return _buffer;
}

auto buffer::load(path const& file, std::any& ctx) noexcept -> load_status
{
    return load(std::make_shared<io::ifstream>(file), io::get_extension(file), ctx);
}

auto buffer::load(std::shared_ptr<io::istream> in, string const& ext, std::any& ctx) noexcept -> load_status
{
    _buffer.clear();
    if (!in || !(*in)) { return load_status::Error; }

    auto decoder {locate_service<decoder::factory>().create_from_sig_or_ext(*in, ext)};
    if (decoder) {
        if (auto info {decoder->open(std::move(in), ctx)}) {
            _info = *info;
            decoder->seek_from_start(milliseconds {0});
            if (auto data {decoder->decode(_info.Channels * _info.FrameCount)}) {
                _buffer = std::move(*data);
                return load_status::Ok;
            }
        }
    }

    return load_status::Error;
}

auto buffer::load_async(path const& file, std::any& ctx) noexcept -> std::future<load_status>
{
    return locate_service<task_manager>().run_async<load_status>([&, file, ctx]() mutable { return load(file, ctx); });
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

auto buffer::Create(information const& info, std::span<f32> data) -> buffer
{
    buffer retValue;
    retValue._info   = info;
    retValue._buffer = {data.begin(), data.end()};
    return retValue;
}

////////////////////////////////////////////////////////////

using namespace std::chrono_literals;

decoder::decoder()  = default;
decoder::~decoder() = default;

auto decoder::open(std::shared_ptr<io::istream> in, std::any& ctx) -> std::optional<buffer::information>
{
    _stream = std::move(in);
    _ctx    = ctx;
    _info   = open();
    return _info;
}

auto decoder::decode(isize size) -> std::optional<std::vector<f32>>
{
    if (!_info || size <= 0) { return std::nullopt; }

    std::vector<f32> buffer(static_cast<usize>(size));
    return decode(buffer) > 0 ? std::optional<std::vector<f32>> {buffer} : std::nullopt;
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
