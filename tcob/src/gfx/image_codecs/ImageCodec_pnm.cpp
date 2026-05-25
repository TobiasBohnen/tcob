// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "ImageCodec_pnm.hpp"

#include <algorithm>
#include <cassert>
#include <cctype>
#include <optional>
#include <span>
#include <vector>

#include "tcob/core/StringUtils.hpp"
#include "tcob/core/io/Stream.hpp"
#include "tcob/gfx/Image.hpp"

namespace tcob::gfx::detail {

static auto CheckSupported(pnm::header const& h) -> bool
{
    return h.FormatString[0] == 'P' && (h.FormatString[1] == '1' || h.FormatString[1] == '2' || h.FormatString[1] == '3');
}

static auto SkipSpaceReadChar(io::istream& reader) -> char
{
    char retValue {0};

    do {
        retValue = reader.read<char>();
    } while (std::isspace(retValue));

    if (retValue == '#') {
        while (retValue != '\n') {
            retValue = reader.read<char>();
        }

        return SkipSpaceReadChar(reader);
    }

    return retValue;
}

template <typename T>
auto read_int(io::istream& reader) -> T
{
    auto const   pre {SkipSpaceReadChar(reader)};
    string const str {reader.read_string_until([](char c) { return std::isspace(c); })};
    return *helper::to_number<T>(pre + str);
}

static auto P1(io::istream& reader, int width, int height) -> std::vector<u8>
{
    // black and white
    std::vector<u8> retValue;

    for (i32 i {0}; i < width * height; ++i) {
        char const pix {SkipSpaceReadChar(reader)};
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

static auto P2(io::istream& reader, pnm::header const& h) -> std::vector<u8>
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

static auto P3(io::istream& reader, pnm::header const& h) -> std::vector<u8>
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
            imgData = P1(in, _header.Width, _header.Height);
            break;
        case pnm::format::P2:
            imgData = P2(in, _header);
            break;
        case pnm::format::P3:
            imgData = P3(in, _header);
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
    if (_header.Width > MAX_SIZE || _header.Height > MAX_SIZE) { return std::nullopt; }

    return CheckSupported(_header)
        ? std::optional {image::information {.Size = {static_cast<i32>(_header.Width), static_cast<i32>(_header.Height)}, .Format = image::format::RGB}}
        : std::nullopt;
}
}
