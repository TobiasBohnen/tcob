// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "ImageCodec_pnm.hpp"

#include <algorithm>

#include "tcob/core/io/Stream.hpp"

namespace tcob::gfx::detail {

auto static check_supported_format(pnm::header const& h) -> bool
{
    return h.FormatString[0] == 'P' && (h.FormatString[1] == '1' || h.FormatString[1] == '2' || h.FormatString[1] == '3');
}

auto static read_until_space(io::istream& reader, std::span<byte> buffer, i32 offset) -> i32
{
    i32 read {0};

    u8 b {reader.read<u8>()};
    while (!std::isspace(b)) {
        buffer[offset + read] = static_cast<byte>(b);
        read++;
        if (reader.is_eof()) {
            break;
        }
        b = reader.read<u8>();
    }

    return read;
}

auto static read_char(io::istream& reader) -> u8
{
    u8 retValue {0};

    do {
        retValue = reader.read<u8>();
    } while (std::isspace(retValue));

    if (retValue == '#') {
        while (retValue != '\n') {
            retValue = reader.read<u8>();
        }

        return read_char(reader);
    }

    return retValue;
}

template <typename T>
auto read_int(io::istream& reader) -> T
{
    std::array<byte, 256> buffer {};
    buffer[0] = read_char(reader);
    i32    read {read_until_space(reader, buffer, 1) + 1};
    string str(buffer.begin(), buffer.begin() + read);

    return static_cast<T>(std::strtol(str.data(), nullptr, 10));
}

auto static read_p1_data(io::istream& reader, int width, int height) -> std::vector<u8>
{
    // black and white
    std::vector<u8> retValue;

    for (i32 i {0}; i < width * height; ++i) {
        u8 const pix {read_char(reader)};
        if (pix == '0') {
            retValue.push_back(255);
            retValue.push_back(255);
            retValue.push_back(255);
        } else {
            retValue.push_back(0);
            retValue.push_back(0);
            retValue.push_back(0);
        }
    }

    return retValue;
}

auto static read_p2_data(io::istream& reader, pnm::header const& h) -> std::vector<u8>
{
    // grayscale
    std::vector<u8> retValue;

    for (u32 i {0}; i < h.Width * h.Height; ++i) {
        u8 const val {std::min(read_int<u8>(reader), static_cast<u8>(h.MaxValue))};
        u8 const pix {static_cast<u8>(static_cast<f32>(val) / h.MaxValue * 255.0f)};
        for (i32 j {0}; j < 3; ++j) {
            retValue.push_back(pix);
        }
    }

    return retValue;
}

auto static read_p3_data(io::istream& reader, pnm::header const& h) -> std::vector<u8>
{
    // RGB
    std::vector<u8> retValue;

    for (u32 i {0}; i < h.Width * h.Height; ++i) {
        for (i32 j {0}; j < 3; ++j) {
            u8 const val {std::min(read_int<u8>(reader), static_cast<u8>(h.MaxValue))};
            u8 const pix {static_cast<u8>(static_cast<f32>(val) / h.MaxValue * 255.0f)};
            retValue.push_back(pix);
        }
    }

    return retValue;
}

void pnm::header::read(io::istream& reader)
{
    FormatString = reader.read_string(2);
    IsAscii      = FormatString[1] < '4';

    Format = static_cast<pnm::format>(FormatString[1] - '0');

    Width  = read_int<u32>(reader);
    Height = read_int<u32>(reader);

    if (Format != pnm::format::P1 && Format != pnm::format::P4) {
        MaxValue = read_int<i16>(reader);
    } else {
        MaxValue = 1;
    }
}

////////////////////////////////////////////////////////////

auto pnm_decoder::decode(io::istream& in) -> std::optional<image>
{
    if (decode_info(in)) {
        std::vector<u8> imgData;
        switch (_header.Format) {
        case pnm::format::P1:
            imgData = read_p1_data(in, _header.Width, _header.Height);
            break;
        case pnm::format::P2:
            imgData = read_p2_data(in, _header);
            break;
        case pnm::format::P3:
            imgData = read_p3_data(in, _header);
            break;
        default:
            break;
        }

        assert(imgData.size() == _header.Width * _header.Height * pnm::BPP);
        return image::Create({static_cast<i32>(_header.Width), static_cast<i32>(_header.Height)}, image::format::RGB, imgData);
    }

    return std::nullopt;
}

auto pnm_decoder::decode_info(io::istream& in) -> std::optional<image::information>
{
    _header.read(in);
    return check_supported_format(_header)
        ? std::optional {image::information {{static_cast<i32>(_header.Width), static_cast<i32>(_header.Height)}, image::format::RGB}}
        : std::nullopt;
}
}
