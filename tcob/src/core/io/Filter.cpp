// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/core/io/Filter.hpp"

#include <cctype>

#include <miniz.h>

namespace tcob::io {

////////////////////////////////////////////////////////////

zlib_filter::zlib_filter(i32 complevel)
    : _level {complevel}
{
}

auto zlib_filter::to(std::span<ubyte const> bytes) const -> std::optional<std::vector<ubyte>>
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
        return std::nullopt;
    }

    mz_deflateEnd(&stream);
    return retValue;
}

auto zlib_filter::from(std::span<ubyte const> bytes) const -> std::optional<std::vector<ubyte>>
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
        return std::nullopt;
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

auto base64_filter::to(std::span<ubyte const> bytes) const -> std::optional<std::vector<ubyte>>
{
    static string const base64_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

    ubyte const*         buf {bytes.data()};
    isize                bufLen {std::ssize(bytes)};
    i32                  i {0};
    std::array<ubyte, 3> charArray3 {};
    std::array<ubyte, 4> charArray4 {};

    std::vector<ubyte> retValue {};
    retValue.reserve(((4 * bufLen / 3) + 3) & ~3);

    while (bufLen--) {
        charArray3[i++] = *(buf++);
        if (i == 3) {
            charArray4[0] = (charArray3[0] & 0xfc) >> 2;
            charArray4[1] = static_cast<ubyte>((charArray3[0] & 0x03) << 4) + ((charArray3[1] & 0xf0) >> 4);
            charArray4[2] = static_cast<ubyte>((charArray3[1] & 0x0f) << 2) + ((charArray3[2] & 0xc0) >> 6);
            charArray4[3] = charArray3[2] & 0x3f;

            for (i = 0; i < 4; i++) {
                retValue.push_back(base64_chars[charArray4[i]]);
            }
            i = 0;
        }
    }

    if (i) {
        for (i32 j {i}; j < 3; j++) {
            charArray3[j] = '\0';
        }

        charArray4[0] = (charArray3[0] & 0xfc) >> 2;
        charArray4[1] = static_cast<ubyte>(((charArray3[0] & 0x03) << 4) + ((charArray3[1] & 0xf0) >> 4));
        charArray4[2] = static_cast<ubyte>(((charArray3[1] & 0x0f) << 2) + ((charArray3[2] & 0xc0) >> 6));
        charArray4[3] = charArray3[2] & 0x3f;

        for (i32 j {0}; (j < i + 1); j++) {
            retValue.push_back(base64_chars[charArray4[j]]);
        }

        while ((i++ < 3)) {
            retValue.push_back('=');
        }
    }

    return retValue;
}

auto base64_filter::from(std::span<ubyte const> bytes) const -> std::optional<std::vector<ubyte>>
{
    static string const base64_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

    isize                bufLen {std::ssize(bytes)};
    i32                  i {0};
    i32                  in {0};
    std::array<ubyte, 3> charArray3 {};
    std::array<ubyte, 4> charArray4 {};

    std::vector<ubyte> retValue {};
    retValue.reserve(3 * (bufLen / 4));

    while (bufLen-- && (bytes[in] != '=') && is_base64(bytes[in])) {
        charArray4[i++] = bytes[in];
        in++;
        if (i == 4) {
            for (i = 0; i < 4; i++) {
                charArray4[i] = static_cast<ubyte>(base64_chars.find(charArray4[i]));
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
        for (i32 j {i}; j < 4; j++) {
            charArray4[j] = 0;
        }

        for (i32 j {0}; j < 4; j++) {
            charArray4[j] = static_cast<ubyte>(base64_chars.find(charArray4[j]));
        }

        charArray3[0] = static_cast<ubyte>((charArray4[0] << 2) + ((charArray4[1] & 0x30) >> 4));
        charArray3[1] = static_cast<ubyte>(((charArray4[1] & 0xf) << 4) + ((charArray4[2] & 0x3c) >> 2));
        charArray3[2] = static_cast<ubyte>(((charArray4[2] & 0x3) << 6) + charArray4[3]);

        for (i32 j {0}; j < i - 1; j++) {
            retValue.push_back(charArray3[j]);
        }
    }

    return retValue;
}

////////////////////////////////////////////////////////////

auto reverser_filter::to(std::span<ubyte const> bytes) const -> std::optional<std::vector<ubyte>>
{
    std::vector<ubyte> retValue {bytes.begin(), bytes.end()};
    std::reverse(retValue.begin(), retValue.end());
    return retValue;
}

auto reverser_filter::from(std::span<ubyte const> bytes) const -> std::optional<std::vector<ubyte>>
{
    return to(bytes);
}

} // namespace io
