// Copyright (c) 2023 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "AudioCodec_bsa.hpp"

////////////////////////////////////////////////////////////

namespace tcob::audio::detail {

constexpr std::array<ubyte, 3> SIGNATURE {'B', 'S', 'A'};
constexpr std::streamsize      OFFSET {sizeof(SIGNATURE) + sizeof(u8) + sizeof(u32) + sizeof(u32)};

void bsa_decoder::seek_from_start(milliseconds pos)
{
    f64 const offset {pos.count() / 1000 * _info.SampleRate / _info.Channels};
    get_stream().seek(static_cast<u64>(offset) + OFFSET, io::seek_dir::Begin);
}

auto bsa_decoder::open() -> std::optional<buffer::info>
{
    auto&                reader {get_stream()};
    std::array<ubyte, 3> sig {};
    reader.read_to<ubyte>(sig);

    if (sig == SIGNATURE) {
        _info.Channels   = reader.read<u8>();
        _info.FrameCount = reader.read<u32>(std::endian::little);
        _info.SampleRate = reader.read<u32>(std::endian::little);
        return _info;
    }

    return std::nullopt;
}

auto bsa_decoder::decode(std::span<f32> outputSamples) -> i32
{
    std::vector<i16> buffer;
    buffer.resize(outputSamples.size());
    get_stream().read_to<i16>(buffer);
    for (usize i {0}; i < outputSamples.size(); ++i) {
        outputSamples[i] = static_cast<f32>(buffer[i]) / std::numeric_limits<i16>::max();
    }
    return static_cast<i32>(std::ssize(outputSamples) / _info.Channels);
}

////////////////////////////////////////////////////////////

auto bsa_encoder::encode(std::span<f32 const> samples, buffer::info const& info, ostream& out) const -> bool
{
    out.write(SIGNATURE);
    out.write<u8>(static_cast<u8>(info.Channels));
    out.write<u32>(static_cast<u32>(info.FrameCount), std::endian::little);
    out.write<u32>(static_cast<u32>(info.SampleRate), std::endian::little);

    std::vector<i16> buffer;
    buffer.resize(samples.size());
    for (usize i {0}; i < samples.size(); ++i) {
        buffer[i] = static_cast<i16>(samples[i] * std::numeric_limits<i16>::max());
    }
    out.write<i16>(buffer);

    return true;
}

}
