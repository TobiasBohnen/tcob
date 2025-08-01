// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "ImageCodec_qoi.hpp"

#if defined(TCOB_ENABLE_FILETYPES_GFX_QOI)

    #include <array>
    #include <bit>
    #include <cstdlib>
    #include <optional>
    #include <vector>

    #include "tcob/core/io/Stream.hpp"
    #include "tcob/gfx/Image.hpp"

namespace tcob::gfx::detail {

constexpr i32 HEADERSIZE {13};

auto qoi_decoder::decode(io::istream& in) -> std::optional<image>
{
    if (auto info {decode_info(in)}) {
        in.seek(-HEADERSIZE, io::seek_dir::Current);
        std::vector<u8> buf {in.read_all<u8>()};

        qoi_desc desc;
        auto*    data {qoi_decode(buf.data(), static_cast<i32>(buf.size()), &desc, 0)};
        image    img {image::Create(info->Size, info->Format,
                                    {static_cast<u8*>(data), static_cast<usize>(info->size_in_bytes())})};
        free(data); // NOLINT(cppcoreguidelines-owning-memory)
        return img;
    }

    return std::nullopt;
}

auto qoi_decoder::decode_info(io::istream& in) -> std::optional<image::information>
{
    std::array<u8, 4> buf {};
    in.read_to<u8>(buf);
    if (buf != std::array<u8, 4> {'q', 'o', 'i', 'f'}) { return std::nullopt; }

    i32 const w {static_cast<i32>(in.read<u32, std::endian::big>())};
    i32 const h {static_cast<i32>(in.read<u32, std::endian::big>())};
    u8 const  bpp {in.read<u8>()};
    return image::information {.Size = {w, h}, .Format = bpp == 3 ? image::format::RGB : image::format::RGBA};
}

////////////////////////////////////////////////////////////

auto qoi_encoder::encode(image const& image, io::ostream& out) const -> bool
{
    auto const& info {image.info()};
    qoi_desc    desc;
    desc.channels   = static_cast<u8>(info.bytes_per_pixel());
    desc.colorspace = QOI_LINEAR;
    desc.height     = info.Size.Height;
    desc.width      = info.Size.Width;

    i32 outputSize {0};
    if (auto* output {qoi_encode(image.ptr(), &desc, &outputSize)}) {
        out.write<u8>({static_cast<u8*>(output), static_cast<usize>(outputSize)});
        free(output); // NOLINT(cppcoreguidelines-owning-memory)
        return true;
    }

    return false;
}

}

#endif
