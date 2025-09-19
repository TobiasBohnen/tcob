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
using pcm_t = i16;

void bsa_decoder::seek_from_start(milliseconds pos)
{
    f64 const samples {pos.count() / 1000.0 * _info.Specs.SampleRate * _info.Specs.Channels};
    f64 const bytes {samples * sizeof(pcm_t)};
    stream().seek(static_cast<std::streamoff>(bytes + OFFSET), io::seek_dir::Begin);
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
    std::vector<pcm_t> buffer(outputSamples.size());
    stream().read_to<pcm_t>(buffer);
    for (usize i {0}; i < outputSamples.size(); ++i) {
        outputSamples[i] = static_cast<f32>(buffer[i]) / std::numeric_limits<pcm_t>::max();
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

    std::vector<pcm_t> buffer(samples.size());
    for (usize i {0}; i < samples.size(); ++i) {
        buffer[i] = static_cast<pcm_t>(samples[i] * std::numeric_limits<pcm_t>::max());
    }
    out.write<pcm_t>(buffer);

    return true;
}

}
