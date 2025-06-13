// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "AudioCodec_bsa.hpp"

#include <array>
#include <bit>
#include <ios>
#include <iterator>
#include <limits>
#include <optional>
#include <span>
#include <vector>

#include "tcob/audio/Buffer.hpp"
#include "tcob/core/io/Stream.hpp"

////////////////////////////////////////////////////////////

namespace tcob::audio::detail {

constexpr std::array<byte, 3> SIGNATURE {'B', 'S', 'A'};
constexpr std::streamsize     OFFSET {sizeof(SIGNATURE) + sizeof(u8) + sizeof(u32) + sizeof(u32)};

void bsa_decoder::seek_from_start(milliseconds pos)
{
    f64 const offset {pos.count() / 1000 * _info.Specs.SampleRate / _info.Specs.Channels};
    stream().seek(static_cast<std::streamoff>(offset + OFFSET), io::seek_dir::Begin);
}

auto bsa_decoder::open() -> std::optional<buffer::information>
{
    auto&               reader {stream()};
    std::array<byte, 3> sig {};
    reader.read_to<byte>(sig);

    if (sig == SIGNATURE) {
        _info.Specs.Channels   = reader.read<u8>();
        _info.FrameCount       = reader.read<u32, std::endian::little>();
        _info.Specs.SampleRate = static_cast<i32>(reader.read<u32, std::endian::little>());
        return _info;
    }

    return std::nullopt;
}

auto bsa_decoder::decode(std::span<f32> outputSamples) -> isize
{
    std::vector<i16> buffer(outputSamples.size());
    stream().read_to<i16>(buffer);
    for (usize i {0}; i < outputSamples.size(); ++i) {
        outputSamples[i] = static_cast<f32>(buffer[i]) / std::numeric_limits<i16>::max();
    }
    return std::ssize(outputSamples);
}

////////////////////////////////////////////////////////////

auto bsa_encoder::encode(std::span<f32 const> samples, buffer::information const& info, io::ostream& out) const -> bool
{
    out.write(SIGNATURE);
    out.write<u8>(static_cast<u8>(info.Specs.Channels));
    out.write<u32, std::endian::little>(static_cast<u32>(info.FrameCount));
    out.write<u32, std::endian::little>(static_cast<u32>(info.Specs.SampleRate));

    std::vector<i16> buffer(samples.size());
    for (usize i {0}; i < samples.size(); ++i) {
        buffer[i] = static_cast<i16>(samples[i] * std::numeric_limits<i16>::max());
    }
    out.write<i16>(buffer);

    return true;
}

}
