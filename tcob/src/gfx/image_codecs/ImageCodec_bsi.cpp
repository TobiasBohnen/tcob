// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "ImageCodec_bsi.hpp"

#include "tcob/core/io/Filter.hpp"
#include "tcob/core/io/Stream.hpp"

namespace tcob::gfx::detail {

void bsi::header::read(io::istream& reader)
{
    reader.read_to<ubyte>(Sig);
    Size   = {static_cast<i32>(reader.read<u32>()), static_cast<i32>(reader.read<u32, std::endian::little>())};
    Format = static_cast<image::format>(reader.read<u8>());
}

constexpr std::array<ubyte, 3> SIGNATURE {'B', 'S', 'I'};

////////////////////////////////////////////////////////////

auto bsi_decoder::decode(io::istream& in) -> std::optional<image>
{
    if (auto info {decode_info(in)}) {
        auto const pixels {in.read_filtered<ubyte>(in.size_in_bytes(), io::zlib_filter {})};
        if (std::ssize(pixels) == info->size_in_bytes()) {
            return image::Create(info->Size, info->Format, pixels);
        }
    }

    return std::nullopt;
}

auto bsi_decoder::decode_info(io::istream& in) -> std::optional<image::info>
{
    bsi::header header {};
    header.read(in);
    return header.Sig == SIGNATURE
        ? std::optional {image::info {header.Size, header.Format}}
        : std::nullopt;
}

////////////////////////////////////////////////////////////

auto bsi_encoder::encode(image const& img, io::ostream& out) const -> bool
{
    out.write(SIGNATURE);
    auto const& info {img.get_info()};
    out.write<u32, std::endian::little>(info.Size.Width);
    out.write<u32, std::endian::little>(info.Size.Height);
    out.write<u8>(static_cast<u8>(info.Format));

    return out.write_filtered<ubyte>(img.buffer(), io::zlib_filter {}) > 0;
}

}
