// Copyright (c) 2023 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "ImageCodec_webp.hpp"

#if defined(TCOB_ENABLE_FILETYPES_GFX_WEBP)

    #include <algorithm>

namespace tcob::gfx::detail {

auto webp_decoder::decode(istream& in) -> std::optional<image>
{
    if (auto info {decode_header(in)}) {
        u8* data {nullptr};
        if (info->Format == image::format::RGBA) {
            data = WebPDecodeRGBA(_buffer.data(), _buffer.size(), nullptr, nullptr);
        } else if (info->Format == image::format::RGB) {
            data = WebPDecodeRGB(_buffer.data(), _buffer.size(), nullptr, nullptr);
        }

        if (data) {
            image img {image::Create(info->Size, info->Format, {data, static_cast<usize>(info->size_in_bytes())})};
            WebPFree(data);
            return img;
        }
    }

    return std::nullopt;
}

auto webp_decoder::decode_header(istream& in) -> std::optional<image::info>
{
    _buffer = in.read_all<u8>();

    WebPBitstreamFeatures features;
    if (WebPGetFeatures(_buffer.data(), _buffer.size(), &features) == VP8_STATUS_OK) {
        return image::info {{features.width, features.height}, features.has_alpha ? image::format::RGBA : image::format::RGB};
    }

    return std::nullopt;
}

////////////////////////////////////////////////////////////

auto webp_encoder::encode(image const& image, ostream& out) const -> bool
{
    auto const& info {image.get_info()};
    auto const  imageBuffer {image.get_data()};
    u8*         output {};
    if (info.Format == image::format::RGBA) {
        usize const outputSize {WebPEncodeLosslessRGBA(imageBuffer.data(), info.Size.Width, info.Size.Height, info.stride(), &output)};
        out.write<u8>({output, outputSize});
    } else if (info.Format == image::format::RGB) {
        usize const outputSize {WebPEncodeLosslessRGB(imageBuffer.data(), info.Size.Width, info.Size.Height, info.stride(), &output)};
        out.write<u8>({output, outputSize});
    }

    WebPFree(output);

    return true;
}

////////////////////////////////////////////////////////////

webp_anim_decoder::webp_anim_decoder()
    : _data {new WebPData}
{
}

webp_anim_decoder::~webp_anim_decoder()
{
    WebPDataClear(_data);
    delete _data;

    if (_decoder) {
        WebPAnimDecoderDelete(_decoder);
    }
}

auto webp_anim_decoder::open() -> std::optional<image::info>
{
    if (_decoder) { return std::nullopt; }

    WebPDataInit(_data);
    auto buffer {get_stream().read_all<u8>()};
    u8*  buf {reinterpret_cast<u8*>(WebPMalloc(buffer.size()))};
    std::copy(buffer.begin(), buffer.end(), buf);
    _data->bytes = buf;
    _data->size  = buffer.size();

    WebPAnimDecoderOptions dec_options;
    WebPAnimDecoderOptionsInit(&dec_options);
    dec_options.color_mode  = WEBP_CSP_MODE::MODE_RGBA;
    dec_options.use_threads = true;

    _decoder = WebPAnimDecoderNew(_data, &dec_options);
    if (_decoder) {
        WebPAnimInfo anim_info;
        WebPAnimDecoderGetInfo(_decoder, &anim_info);
        _size = {static_cast<i32>(anim_info.canvas_width), static_cast<i32>(anim_info.canvas_height)};
        return image::info {_size, image::format::RGBA};
    }

    return std::nullopt;
}

auto webp_anim_decoder::get_current_frame() const -> u8 const*
{
    return _buffer;
}

void webp_anim_decoder::reset()
{
    _currentTimeStamp = 0;
    if (_decoder) {
        WebPAnimDecoderReset(_decoder);
    }
}

auto webp_anim_decoder::seek_from_current(milliseconds ts) -> animated_image_decoder::status
{
    auto timestamp {static_cast<i32>(ts.count())};
    if (!_decoder) {
        return animated_image_decoder::status::DecodeFailure;
    }

    if (!WebPAnimDecoderHasMoreFrames(_decoder)) {
        return animated_image_decoder::status::NoMoreFrames;
    }

    if (timestamp <= _currentTimeStamp) {
        return animated_image_decoder::status::OldFrame;
    }

    while (timestamp > _currentTimeStamp) {
        if (!WebPAnimDecoderGetNext(_decoder, &_buffer, &_currentTimeStamp)) {
            return animated_image_decoder::status::DecodeFailure;
        }
        if (!WebPAnimDecoderHasMoreFrames(_decoder)) {
            return animated_image_decoder::status::NewFrame;
        }
    }

    return animated_image_decoder::status::NewFrame;
}

////////////////////////////////////////////////////////////

webp_anim_encoder::webp_anim_encoder() = default;

webp_anim_encoder::~webp_anim_encoder()
{
    if (_encoder) {
        WebPAnimEncoderDelete(_encoder);
    }
}

auto webp_anim_encoder::encode(std::span<frame const> frames, ostream& out) -> bool
{
    bool retValue {true};

    WebPPicture pic;
    WebPPictureInit(&pic);

    for (auto const& frame : frames) {
        if (!retValue) {
            break;
        }

        auto const& info {frame.Image.get_info()};
        if (!_encoder) {
            _encoder = WebPAnimEncoderNew(info.Size.Width, info.Size.Height, nullptr);
            _imgSize = info.Size;

            pic.width  = _imgSize.Width;
            pic.height = _imgSize.Height;
        } else if (_imgSize != info.Size) {
            retValue = false;
            break;
        }

        if (info.Format == image::format::RGBA) {
            retValue = WebPPictureImportRGBA(&pic, frame.Image.get_data().data(), info.stride());
        } else if (info.Format == image::format::RGB) {
            retValue = WebPPictureImportRGB(&pic, frame.Image.get_data().data(), info.stride());
        }

        if (retValue) {
            retValue = WebPAnimEncoderAdd(_encoder, &pic, static_cast<i32>(frame.TimeStamp.count()), nullptr) != 0;
        }
    }

    WebPPictureFree(&pic);

    if (!retValue) {
        return false;
    }

    WebPData data;
    retValue = WebPAnimEncoderAssemble(_encoder, &data) != 0;
    if (retValue) {
        out.write<u8>({data.bytes, data.size});
    }

    WebPDataClear(&data);
    return retValue;
}
}

#endif
