// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "AudioCodec_wav.hpp"

#if defined(TCOB_ENABLE_FILETYPES_AUDIO_DRLIBS)

    #include "tcob/core/io/Stream.hpp"

////////////////////////////////////////////////////////////

namespace tcob::audio::detail {

extern "C" {
auto static read_wav(void* userdata, void* buffer, usize bytesToRead) -> usize
{
    auto* stream {static_cast<io::istream*>(userdata)};
    return static_cast<usize>(stream->read_to<byte>({static_cast<byte*>(buffer), bytesToRead}));
}

auto static write_wav(void* userdata, void const* buffer, usize bytesToWrite) -> usize
{
    auto*      stream {static_cast<io::ostream*>(userdata)};
    auto const retValue {stream->write<byte>({static_cast<byte const*>(buffer), bytesToWrite})};
    return static_cast<usize>(std::max<isize>(0, retValue));
}

auto static seek_wav(void* userdata, i32 offset, drwav_seek_origin origin) -> drwav_bool32
{
    auto*      stream {static_cast<io::istream*>(userdata)};
    auto const dir {origin == drwav_seek_origin_current ? io::seek_dir::Current : io::seek_dir::Begin};
    return stream->seek(offset, dir);
}
}

////////////////////////////////////////////////////////////

wav_decoder::~wav_decoder()
{
    drwav_uninit(&_wav);
}

void wav_decoder::seek_from_start(milliseconds pos)
{
    f64 const offset {pos.count() / 1000 * _info.SampleRate / _info.Channels};
    drwav_seek_to_pcm_frame(&_wav, static_cast<u64>(offset));
}

auto wav_decoder::open() -> std::optional<buffer::information>
{
    if (drwav_init(&_wav, &read_wav, &seek_wav, &stream(), nullptr)) {
        _info.Channels   = _wav.channels;
        _info.SampleRate = static_cast<i32>(_wav.sampleRate);
        _info.FrameCount = static_cast<i64>(_wav.totalPCMFrameCount);
        return _info;
    }

    return std::nullopt;
}

auto wav_decoder::decode(std::span<f32> outputSamples) -> i32
{
    u64 const wantRead {outputSamples.size() / static_cast<u32>(_info.Channels)};
    return static_cast<i32>(drwav_read_pcm_frames_f32(&_wav, wantRead, outputSamples.data()));
}

////////////////////////////////////////////////////////////

auto wav_encoder::encode(std::span<f32 const> samples, buffer::information const& info, io::ostream& out) const -> bool
{
    drwav_data_format format;
    format.format        = DR_WAVE_FORMAT_PCM;
    format.bitsPerSample = 16;
    format.sampleRate    = static_cast<u32>(info.SampleRate);
    format.channels      = static_cast<u32>(info.Channels);
    format.container     = drwav_container_riff;

    std::vector<drwav_int16> pcms(samples.size());
    drwav_f32_to_s16(pcms.data(), samples.data(), samples.size());

    drwav wav;
    drwav_init_write_sequential_pcm_frames(&wav, &format, static_cast<u64>(info.FrameCount), &write_wav, &out, nullptr);
    drwav_write_pcm_frames(&wav, static_cast<u64>(info.FrameCount), pcms.data());
    drwav_uninit(&wav);
    return true;
}
}

#endif
