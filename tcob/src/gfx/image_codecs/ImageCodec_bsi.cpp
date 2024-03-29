// Copyright (c) 2023 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "ImageCodec_bsi.hpp"

#include "tcob/core/io/Filter.hpp"

namespace tcob::gfx::detail {

void bsi::header::read(istream& reader)
{
    reader.read_to<ubyte>(Sig);
    Size   = {static_cast<i32>(reader.read<u32>(std::endian::little)), static_cast<i32>(reader.read<u32>(std::endian::little))};
    Format = static_cast<image::format>(reader.read<u8>());
}

constexpr std::array<ubyte, 3> SIGNATURE {'B', 'S', 'I'};

////////////////////////////////////////////////////////////

auto bsi_decoder::decode(istream& in) -> std::optional<image>
{
    if (auto info {decode_header(in)}) {
        auto const buffer {in.read_all<ubyte>()};
        if (auto const pixels {io::zlib_filter {}.from(buffer)}) {
            if (std::ssize(*pixels) == info->size_in_bytes()) {
                return image::Create(info->Size, info->Format, *pixels);
            }
        }
    }

    return std::nullopt;
}

auto bsi_decoder::decode_header(istream& in) -> std::optional<image::info>
{
    bsi::header header {};
    header.read(in);
    return header.Sig == SIGNATURE
        ? std::optional {image::info {header.Size, header.Format}}
        : std::nullopt;
}

////////////////////////////////////////////////////////////

auto bsi_encoder::encode(image const& img, ostream& out) const -> bool
{
    out.write(SIGNATURE);
    auto const& info {img.get_info()};
    out.write<u32>(info.Size.Width, std::endian::little);
    out.write<u32>(info.Size.Height, std::endian::little);
    out.write<u8>(static_cast<u8>(info.Format));

    if (auto buf {io::zlib_filter {}.to(img.get_data())}) {
        out.write<ubyte>(*buf);
        return true;
    }
    return false;
}

}
