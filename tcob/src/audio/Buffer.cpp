// Copyright (c) 2023 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/audio/Buffer.hpp"

#include "tcob/audio/AudioSource.hpp"
#include "tcob/core/Semaphore.hpp"
#include "tcob/core/ServiceLocator.hpp"
#include "tcob/core/io/FileStream.hpp"
#include "tcob/core/io/FileSystem.hpp"

namespace tcob::audio {

auto buffer::get_info() const -> info const&
{
    return _info;
}

auto buffer::get_data() const -> std::span<f32 const>
{
    return _buffer;
}

auto buffer::get_data() -> std::span<f32>
{
    return _buffer;
}

auto buffer::load(path const& file, std::any& ctx) noexcept -> load_status
{
    if (!io::is_file(file)) { return load_status::FileNotFound; }

    return load(std::make_shared<io::ifstream>(file), io::get_extension(file), ctx);
}

auto buffer::load(std::shared_ptr<istream> in, string const& ext, std::any& ctx) noexcept -> load_status
{
    auto decoder {locate_service<decoder::factory>().create_from_sig_or_ext(*in, ext)};
    if (decoder) {
        if (auto info {decoder->open(std::move(in), ctx)}) {
            _info = *info;
            _buffer.resize(_info.Channels * _info.FrameCount);
            decoder->seek_from_start(milliseconds {0});
            if (decoder->decode_to_buffer(_buffer)) {
                return load_status::Ok;
            }
        }
    }

    return load_status::Error;
}

auto buffer::load_async(path const& file, std::any& ctx) noexcept -> std::future<load_status>
{
    return std::async(std::launch::async, [&, file, ctx]() mutable {
        auto& sema {locate_service<semaphore>()};
        sema.acquire();
        auto retValue {load(file, ctx)};
        sema.release();
        return retValue;
    });
}

auto buffer::save(path const& file) const -> bool
{
    io::ofstream of {file};
    return save(of, io::get_extension(file));
}

auto buffer::save(ostream& out, string const& ext) const -> bool
{
    if (_info.FrameCount == 0) { return false; }

    if (auto enc {locate_service<encoder::factory>().create(ext)}) {
        return enc->encode(_buffer, _info, out);
    }

    return false;
}

auto buffer::save_async(path const& file) const -> std::future<bool>
{
    std::promise<bool> pro;
    auto               retValue {pro.get_future()};
    std::thread([*this, file](std::promise<bool> p) { p.set_value(save(file)); }, std::move(pro)).detach();
    return retValue;
}

auto buffer::Create(info const& info, std::span<f32> data) -> buffer
{
    buffer retValue;
    retValue._info   = info;
    retValue._buffer = {data.begin(), data.end()};
    return retValue;
}

}
