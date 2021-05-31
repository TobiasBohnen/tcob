// Copyright (c) 2021 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include <tcob/gfx/drawables/WebpAnimation.hpp>

#include <webp/demux.h>

#include <tcob/core/io/FileStream.hpp>
#include <tcob/gfx/gl/GLCapabilities.hpp>

namespace tcob {

WebpAnimation::WebpAnimation()
{
    _texture = std::make_shared<gl::Texture2D>();

    _quad.color(Colors::White);
    _quad.texcoords({ { 0, 0, 1, 1 }, 0 });

    _renderer.set_geometry(&_quad, 1);
}

WebpAnimation::~WebpAnimation() = default;

WebpAnimation::WebpAnimation(const WebpAnimation& other)
{
    *this = other;
}

auto WebpAnimation::operator=(const WebpAnimation& other) -> WebpAnimation&
{
    stop();

    Transformable::operator=(other);
    _decoder = other._decoder;
    _decoder->reset();
    _frameSize = other._frameSize;
    _texture->create(_frameSize);
    material(other._material);
    return *this;
}

auto WebpAnimation::load(const std::string& file) -> bool
{
    stop();

    if (!FileSystem::exists(file))
        return false;

    _decoder = std::make_unique<detail::WebpAnimDecoder>(file);
    _frameSize = { _decoder->size() };
    _texture->create(_frameSize);

    return true;
}

void WebpAnimation::start(bool looped)
{
    if (_isRunning || !_decoder) {
        //TODO: log error
        return;
    }

    _looped = looped;

    _elapsedTime = 0;
    _isRunning = true;
}

void WebpAnimation::restart()
{
    stop();
    start(_looped);
}

void WebpAnimation::toggle_pause()
{
    _isRunning = !_isRunning;
}

void WebpAnimation::stop()
{
    if (_decoder)
        _decoder->reset();
    _elapsedTime = 0;
    _isRunning = false;
}

auto WebpAnimation::is_running() const -> bool
{
    return _isRunning;
}

auto WebpAnimation::material() const -> ResourcePtr<Material>
{
    return _material;
}

void WebpAnimation::material(ResourcePtr<Material> material)
{
    _material = std::move(material);
    if (_texture && _material) {
        _material->Texture = { std::make_shared<Resource<gl::Texture>>(_texture) };
        _renderer.material(_material.object());
    }
}

void WebpAnimation::update(f64 deltaTime)
{
    if (is_transform_dirty()) {
        _quad.position(bounds(), transform());
        _renderer.modify_geometry(&_quad, 1, 0);
    }

    if (!_isRunning || !_decoder)
        return;

    _elapsedTime += deltaTime;

    u8* buffer;
    auto result { _decoder->get_frame(static_cast<i32>(_elapsedTime), &buffer) };

    switch (result) {
    case detail::DecodeResult::NewFrame:
        _texture->update(PointU::Zero, _frameSize, buffer);
        break;
    case detail::DecodeResult::NoMoreFrames:
        if (_looped) {
            _elapsedTime = 0;
            _decoder->reset();
        } else {
            _isRunning = false;
        }
        break;
    default: //TODO: handle failure
        break;
    }
}

void WebpAnimation::draw(gl::RenderTarget& target)
{
    if (!is_visible() || _elapsedTime == 0 || !_material)
        return;

    _renderer.render_to_target(target);
}

}

////////////////////////////////////////////////////////////

namespace tcob::detail {

WebpAnimDecoder::WebpAnimDecoder(const std::string& file)
    : _data { new WebPData }
{
    WebPDataInit(_data);
    InputFileStreamU stream { file };
    auto buffer { stream.read_all() };
    u8* buf { reinterpret_cast<u8*>(WebPMalloc(buffer.size())) };
    std::copy(buffer.begin(), buffer.end(), buf);
    _data->bytes = buf;
    _data->size = buffer.size();

    WebPAnimDecoderOptions dec_options;
    WebPAnimDecoderOptionsInit(&dec_options);
    dec_options.color_mode = WEBP_CSP_MODE::MODE_RGBA;
    dec_options.use_threads = true;

    _dec = WebPAnimDecoderNew(_data, &dec_options);
    if (_dec) {
        WebPAnimInfo anim_info;
        WebPAnimDecoderGetInfo(_dec, &anim_info);
        _size = { anim_info.canvas_width, anim_info.canvas_height };
    }
}

WebpAnimDecoder::~WebpAnimDecoder()
{
    WebPDataClear(_data);
    delete _data;

    WebPAnimDecoderDelete(_dec);
}

void WebpAnimDecoder::reset()
{
    _currentTimeStamp = 0;
    WebPAnimDecoderReset(_dec);
}

auto WebpAnimDecoder::get_frame(i32 timestamp, u8** buffer) -> DecodeResult
{
    if (!_dec) {
        return DecodeResult::DecodeFailure;
    }

    if (!WebPAnimDecoderHasMoreFrames(_dec)) {
        return DecodeResult::NoMoreFrames;
    }

    if (timestamp <= _currentTimeStamp) {
        return DecodeResult::OldFrame;
    }

    while (timestamp > _currentTimeStamp) {
        if (!WebPAnimDecoderGetNext(_dec, buffer, &_currentTimeStamp)) {
            return DecodeResult::DecodeFailure;
        }
        if (!WebPAnimDecoderHasMoreFrames(_dec)) {
            return DecodeResult::NewFrame;
        }
    }

    return DecodeResult::NewFrame;
}

auto WebpAnimDecoder::size() const -> SizeU
{
    return _size;
}

}