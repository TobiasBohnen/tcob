// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/core/io/Filter.hpp"

#include <algorithm>
#include <array>
#include <cassert>
#include <cctype>
#include <cstring>
#include <span>
#include <string_view>
#include <vector>

#include <miniz.h>

using namespace std::literals;

namespace tcob::io {

////////////////////////////////////////////////////////////

zlib_filter::zlib_filter(i32 complevel)
    : _level {complevel}
{
}

auto zlib_filter::to(std::span<ubyte const> bytes) const -> std::vector<ubyte>
{
    static constexpr usize BUFFER_SIZE {1024};
    std::vector<ubyte>     retValue;

    mz_stream stream;
    memset(&stream, 0, sizeof(stream));

    stream.next_in  = bytes.data();
    stream.avail_in = static_cast<mz_ulong>(bytes.size());

    i32 status {mz_deflateInit(&stream, _level)};
    while (status == MZ_OK) {
        retValue.resize(retValue.size() + BUFFER_SIZE);
        stream.next_out  = retValue.data() + stream.total_out;
        stream.avail_out = static_cast<u32>(retValue.size() - stream.total_out);
        status           = mz_deflate(&stream, MZ_FINISH);
    }

    if (status == MZ_STREAM_END) {
        retValue.resize(stream.total_out);
    } else {
        mz_deflateEnd(&stream);
        return {};
    }

    mz_deflateEnd(&stream);
    return retValue;
}

auto zlib_filter::from(std::span<ubyte const> bytes) const -> std::vector<ubyte>
{
    static constexpr usize BUFFER_SIZE {1024};
    std::vector<ubyte>     retValue;

    mz_stream stream;
    memset(&stream, 0, sizeof(stream));

    stream.next_in  = bytes.data();
    stream.avail_in = static_cast<mz_ulong>(bytes.size());

    i32 status {mz_inflateInit(&stream)};
    while (status == MZ_OK) {
        retValue.resize(retValue.size() + BUFFER_SIZE);
        stream.next_out  = reinterpret_cast<unsigned char*>(retValue.data() + stream.total_out);
        stream.avail_out = static_cast<u32>(retValue.size() - stream.total_out);
        status           = mz_inflate(&stream, MZ_NO_FLUSH);
    }

    if (status == MZ_STREAM_END) {
        retValue.resize(stream.total_out);
    } else {
        mz_inflateEnd(&stream);
        return {};
    }

    mz_inflateEnd(&stream);
    return retValue;
}

////////////////////////////////////////////////////////////
// based on:https://stackoverflow.com/a/13935718/13220389

static inline auto is_base64(byte c) -> bool
{
    return (isalnum(static_cast<ubyte>(c)) || (c == '+') || (c == '/'));
}

static string_view const base64_chars {"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"sv};

auto base64_filter::to(std::span<ubyte const> bytes) const -> std::vector<ubyte>
{
    ubyte const*         buf {bytes.data()};
    isize                bufLen {std::ssize(bytes)};
    u32                  i {0};
    std::array<ubyte, 3> charArray3 {};
    std::array<ubyte, 4> charArray4 {};

    std::vector<ubyte> retValue {};
    retValue.reserve(static_cast<usize>(((4 * bufLen / 3) + 3) & ~3));

    while (bufLen--) {
        charArray3[i++] = *(buf++);
        if (i == 3) {
            charArray4[0] = (charArray3[0] & 0xfc) >> 2;
            charArray4[1] = static_cast<ubyte>((charArray3[0] & 0x03) << 4) + ((charArray3[1] & 0xf0) >> 4);
            charArray4[2] = static_cast<ubyte>((charArray3[1] & 0x0f) << 2) + ((charArray3[2] & 0xc0) >> 6);
            charArray4[3] = charArray3[2] & 0x3f;

            for (i = 0; i < 4; i++) {
                retValue.push_back(static_cast<ubyte>(base64_chars[charArray4[i]]));
            }
            i = 0;
        }
    }

    if (i) {
        for (u32 j {i}; j < 3; j++) {
            charArray3[j] = '\0';
        }

        charArray4[0] = (charArray3[0] & 0xfc) >> 2;
        charArray4[1] = static_cast<ubyte>(((charArray3[0] & 0x03) << 4) + ((charArray3[1] & 0xf0) >> 4));
        charArray4[2] = static_cast<ubyte>(((charArray3[1] & 0x0f) << 2) + ((charArray3[2] & 0xc0) >> 6));
        charArray4[3] = charArray3[2] & 0x3f;

        for (u32 j {0}; (j < i + 1); j++) {
            retValue.push_back(static_cast<ubyte>(base64_chars[charArray4[j]]));
        }

        while ((i++ < 3)) {
            retValue.push_back('=');
        }
    }

    return retValue;
}

auto base64_filter::from(std::span<ubyte const> bytes) const -> std::vector<ubyte>
{
    isize                bufLen {std::ssize(bytes)};
    u32                  i {0};
    u32                  in {0};
    std::array<ubyte, 3> charArray3 {};
    std::array<ubyte, 4> charArray4 {};

    std::vector<ubyte> retValue {};
    retValue.reserve(static_cast<usize>(3 * (bufLen / 4)));

    while (bufLen-- && (bytes[in] != '=') && is_base64(static_cast<byte>(bytes[in]))) {
        charArray4[i++] = bytes[in];
        in++;
        if (i == 4) {
            for (i = 0; i < 4; i++) {
                charArray4[i] = static_cast<ubyte>(base64_chars.find(static_cast<byte>(charArray4[i])));
            }

            charArray3[0] = static_cast<ubyte>((charArray4[0] << 2) + ((charArray4[1] & 0x30) >> 4));
            charArray3[1] = static_cast<ubyte>(((charArray4[1] & 0xf) << 4) + ((charArray4[2] & 0x3c) >> 2));
            charArray3[2] = static_cast<ubyte>(((charArray4[2] & 0x3) << 6) + charArray4[3]);

            for (i = 0; i < 3; i++) {
                retValue.push_back(charArray3[i]);
            }
            i = 0;
        }
    }

    if (i) {
        for (u32 j {i}; j < 4; j++) {
            charArray4[j] = 0;
        }

        for (u32 j {0}; j < 4; j++) {
            charArray4[j] = static_cast<ubyte>(base64_chars.find(static_cast<byte>(charArray4[j])));
        }

        charArray3[0] = static_cast<ubyte>((charArray4[0] << 2) + ((charArray4[1] & 0x30) >> 4));
        charArray3[1] = static_cast<ubyte>(((charArray4[1] & 0xf) << 4) + ((charArray4[2] & 0x3c) >> 2));
        charArray3[2] = static_cast<ubyte>(((charArray4[2] & 0x3) << 6) + charArray4[3]);

        for (u32 j {0}; j < i - 1; j++) {
            retValue.push_back(charArray3[j]);
        }
    }

    return retValue;
}

////////////////////////////////////////////////////////////

static string_view const          z85_encode {"0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ.-:+=^!/*?&<>()[]{}@%$#"sv};
static std::array<byte, 96> const z85_decode = {0x00, 0x44, 0x00, 0x54, 0x53, 0x52, 0x48, 0x00,
                                                0x4B, 0x4C, 0x46, 0x41, 0x00, 0x3F, 0x3E, 0x45,
                                                0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
                                                0x08, 0x09, 0x40, 0x00, 0x49, 0x42, 0x4A, 0x47,
                                                0x51, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2A,
                                                0x2B, 0x2C, 0x2D, 0x2E, 0x2F, 0x30, 0x31, 0x32,
                                                0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A,
                                                0x3B, 0x3C, 0x3D, 0x4D, 0x00, 0x4E, 0x43, 0x00,
                                                0x00, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10,
                                                0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18,
                                                0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 0x20,
                                                0x21, 0x22, 0x23, 0x4F, 0x00, 0x50, 0x00, 0x00};

auto z85_filter::to(std::span<ubyte const> bytes) const -> std::vector<ubyte>
{
    usize const bufLen {bytes.size()};

    if (bufLen % 4) { return {}; }

    usize const        encodedLen {bufLen * 5 / 4};
    std::vector<ubyte> encoded(encodedLen);

    u32 charNbr {0};
    u32 byteNbr {0};
    u32 value {0};
    while (byteNbr < bufLen) {
        value = value * 256 + bytes[byteNbr++];
        if (byteNbr % 4 == 0) {
            u32 divisor {85 * 85 * 85 * 85};
            while (divisor) {
                encoded[charNbr++] = static_cast<ubyte>(z85_encode[value / divisor % 85]);
                divisor /= 85;
            }
            value = 0;
        }
    }

    assert(charNbr == encodedLen);
    return encoded;
}

auto z85_filter::from(std::span<ubyte const> bytes) const -> std::vector<ubyte>
{
    usize const bufLen {bytes.size()};

    if (bufLen % 5) { return {}; }

    usize const        decodedSize {bufLen * 4 / 5};
    std::vector<ubyte> decoded(decodedSize);

    u32 charNbr {0};
    u32 byteNbr {0};
    u32 value {0};
    while (charNbr < bufLen) {
        value = value * 85 + static_cast<u32>(z85_decode[bytes[charNbr++] - 32]);
        if (charNbr % 5 == 0) {
            u32 divisor {256 * 256 * 256};
            while (divisor) {
                decoded[byteNbr++] = static_cast<ubyte>(value / divisor % 256);
                divisor /= 256;
            }
            value = 0;
        }
    }

    assert(byteNbr == decodedSize);
    return decoded;
}

////////////////////////////////////////////////////////////

auto reverser_filter::to(std::span<ubyte const> bytes) const -> std::vector<ubyte>
{
    std::vector<ubyte> retValue {bytes.begin(), bytes.end()};
    std::ranges::reverse(retValue);
    return retValue;
}

auto reverser_filter::from(std::span<ubyte const> bytes) const -> std::vector<ubyte>
{
    return to(bytes);
}

} // namespace io
