// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/Texture.hpp"

#include <utility>

#include "tcob/core/ServiceLocator.hpp"
#include "tcob/core/io/FileStream.hpp"
#include "tcob/core/io/FileSystem.hpp"
#include "tcob/gfx/RenderSystem.hpp"

namespace tcob::gfx {
using namespace std::chrono_literals;

////////////////////////////////////////////////////////////

texture::texture()
    : Filtering {{[&]() { return _impl->get_filtering(); },
                  [&](filtering value) { _impl->set_filtering(value); }}}
    , Wrapping {{[&]() { return _impl->get_wrapping(); },
                 [&](wrapping value) { _impl->set_wrapping(value); }}}
    , _impl {locate_service<render_system>().create_texture()}
{
}

texture::~texture() = default;

texture::operator bool() const
{
    return _impl->is_valid();
}

auto texture::info() const -> information
{
    return {.Size = _size, .Format = _format, .Depth = _depth};
}

auto texture::copy_to_image(u32 level) const -> image
{
    if (level > _depth) {
        return {};
    }

    return _impl->copy_to_image(level);
}

void texture::add_region(string const& name, texture_region const& region)
{
    _regions[name] = region;
}

auto texture::get_region(string const& name) const -> texture_region const&
{
    return _regions.at(name);
}

auto texture::get_regions() const -> std::unordered_map<string, texture_region> const&
{
    return _regions;
}

auto texture::has_region(string const& name) const -> bool
{
    return _regions.contains(name);
}

void texture::create(size_i size, u32 depth, format f)
{
    if (size == _size && depth == _depth && f == _format) {
        return;
    }

    _impl->create(size, depth, f);
    _size   = size;
    _format = f;
    _depth  = depth;
}

void texture::update_data(void const* data, u32 depth, i32 rowLength, i32 alignment) const
{
    update_data(point_i::Zero, _size, data, depth, rowLength, alignment);
}

void texture::update_data(std::span<u8 const> data, u32 depth, i32 rowLength, i32 alignment) const
{
    update_data(point_i::Zero, _size, data.data(), depth, rowLength, alignment);
}

void texture::update_data(point_i origin, size_i size, void const* data, u32 depth, i32 rowLength, i32 alignment) const
{
    _impl->update(origin, size, data, depth, rowLength, alignment);
}

////////////////////////////////////////////////////////////

auto animated_texture::load(path const& file) noexcept -> load_status
{
    return load(std::make_shared<io::ifstream>(file), io::get_extension(file));
}

auto animated_texture::load(std::shared_ptr<io::istream> in, string const& ext) noexcept -> load_status
{
    if (!in) { return load_status::Error; }
    stop();

    _decoder = locate_service<animated_image_decoder::factory>().create_from_sig_or_ext(*in, ext);
    if (auto info {_decoder->open(std::move(in))}) {
        _frameInfo = *info;
        i32 const bpp {_frameInfo.bytes_per_pixel()};
        if (bpp == 4) {
            create(_frameInfo.Size, 1, format::RGBA8);
        } else if (bpp == 3) {
            create(_frameInfo.Size, 1, format::RGB8);
        } else {
            return load_status::Error;
        }

        return load_status::Ok;
    }

    return load_status::Error;
}

auto animated_texture::status() const -> playback_status
{
    return _status;
}

auto animated_texture::is_looping() const -> bool
{
    return _isLooping;
}

void animated_texture::start(bool looping)
{
    if (status() == playback_status::Stopped) { // start if stopped
        if (_decoder) {
            _isLooping = looping;
            _status    = playback_status::Running;
            reset_decoder();
        }
    } else if (status() == playback_status::Paused) { // resume if paused
        _isLooping = looping;
        resume();
    }
}

void animated_texture::stop()
{
    if (status() != playback_status::Stopped) { // stop if running or paused
        _status = playback_status::Stopped;
        reset_decoder();
    }
}

void animated_texture::restart()
{
    stop();
    start(is_looping());
}

void animated_texture::pause()
{
    if (status() == playback_status::Running) { // pause if running
        _status = playback_status::Paused;
    }
}

void animated_texture::resume()
{
    if (status() == playback_status::Paused) { // resume if paused
        _status = playback_status::Running;
    }
}

void animated_texture::toggle_pause()
{
    status() == playback_status::Paused ? resume() : pause();
}

void animated_texture::reset_decoder()
{
    if (_decoder) {
        _decoder->reset();
    }
    _elapsedTime = 0ms;
}

void animated_texture::on_update(milliseconds deltaTime)
{
    if (status() != playback_status::Running || !_decoder) {
        return;
    }

    _elapsedTime += deltaTime;
    auto const result {_decoder->advance(_elapsedTime)};

    bool updateTexture {false};

    switch (result) {
    case animated_image_decoder::status::OldFrame: break;
    case animated_image_decoder::status::NewFrame: updateTexture = true; break;
    case animated_image_decoder::status::NoMoreFrames:
        if (is_looping()) {
            restart();
        } else {
            stop();
        }
        return;
    default: // TODO: handle failure
        break;
    }

    auto const* textureBuffer {_decoder->current_frame()};
    if (updateTexture && textureBuffer) {
        if (_frameInfo.bytes_per_pixel() == 4) {
            update_data(textureBuffer, 0, 0, 4);
        } else {
            update_data(textureBuffer, 0, 0, 1);
        }
        NewFrame();
    }
}
}
