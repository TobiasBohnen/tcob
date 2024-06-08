// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "AudioCodec_ogg.hpp"

#if defined(TCOB_ENABLE_FILETYPES_AUDIO_VORBIS)

    #include "tcob/core/random/Random.hpp"

////////////////////////////////////////////////////////////

namespace tcob::audio::detail {

extern "C" {
auto static read_vorbis(void* ptr, size_t size, size_t nmemb, void* datasource) -> size_t
{
    auto* stream {static_cast<istream*>(datasource)};
    return static_cast<size_t>(stream->read_to<byte>({reinterpret_cast<byte*>(ptr), static_cast<usize>(size * nmemb)}));
}

auto static seek_vorbis(void* datasource, ogg_int64_t offset, int whence) -> int
{
    auto*      stream {static_cast<istream*>(datasource)};
    auto const dir {static_cast<io::seek_dir>(whence)};
    stream->seek(offset, dir);
    return 0;
}

auto static tell_vorbis(void* datasource) -> long
{
    istream* stream {static_cast<istream*>(datasource)};
    return static_cast<long>(stream->tell());
}

static ov_callbacks vorbisCallbacks {
    .read_func  = &read_vorbis,
    .seek_func  = &seek_vorbis,
    .close_func = nullptr,
    .tell_func  = &tell_vorbis};
}

vorbis_decoder::vorbis_decoder() = default;

vorbis_decoder::~vorbis_decoder()
{
    ov_clear(&_file);
}

void vorbis_decoder::seek_from_start(milliseconds pos)
{
    ov_time_seek(&_file, pos.count());
}

auto vorbis_decoder::open() -> std::optional<buffer::info>
{
    i32 const error {ov_open_callbacks(&get_stream(), &_file, nullptr, 0, vorbisCallbacks)};
    if (error == 0) {
        auto* info {ov_info(&_file, -1)};
        _info.Channels   = info->channels;
        _info.SampleRate = info->rate;
        _info.FrameCount = ov_pcm_total(&_file, -1);
        return _info;
    }

    return std::nullopt;
}

auto vorbis_decoder::decode(std::span<f32> outputSamples) -> i32
{
    f32** pcm {nullptr};
    i32   offsetSamples {0};
    i32   wantFrames {static_cast<i32>(outputSamples.size() / _info.Channels)};
    for (;;) {
        i32 const readFrames {static_cast<i32>(ov_read_float(&_file, &pcm, wantFrames, &_section))};
        if (readFrames <= 0) {
            break;
        }

        for (i32 i {0}; i < readFrames; ++i) {
            for (i32 ch {0}; ch < _info.Channels; ++ch) {
                assert(offsetSamples < std::ssize(outputSamples));
                outputSamples[offsetSamples++] = pcm[ch][i];
            }
        }
        wantFrames -= readFrames;
    }

    return offsetSamples / _info.Channels;
}

////////////////////////////////////////////////////////////

auto vorbis_encoder::encode(std::span<f32 const> samples, buffer::info const& info, ostream& out) const -> bool
{
    ogg_stream_state os; /* take physical pages, weld into a logical stream of packets */
    ogg_page         og; /* one Ogg bitstream page.  Vorbis packets are inside */
    ogg_packet       op; /* one raw packet of data for decode */

    vorbis_info    vi;   /* struct that stores all the static vorbis bitstream settings */
    vorbis_comment vc;   /* struct that stores all the user comments */

    vorbis_dsp_state vd; /* central working state for the packet->PCM decoder */
    vorbis_block     vb; /* local working space for packet->PCM decode */

    vorbis_info_init(&vi);
    i32 const ret {vorbis_encode_init_vbr(&vi, info.Channels, info.SampleRate, 0.5f)};
    if (ret != 0) { return false; }

    vorbis_comment_init(&vc);
    vorbis_comment_add_tag(&vc, "ENCODER", "tcob");

    vorbis_analysis_init(&vd, &vi);
    vorbis_block_init(&vd, &vb);

    random::rng_game_rand rng;
    ogg_stream_init(&os, rng.next());

    {
        ogg_packet header;
        ogg_packet header_comm;
        ogg_packet header_code;

        vorbis_analysis_headerout(&vd, &vc, &header, &header_comm, &header_code);
        ogg_stream_packetin(&os, &header);
        ogg_stream_packetin(&os, &header_comm);
        ogg_stream_packetin(&os, &header_code);

        if (ogg_stream_flush(&os, &og) == 0) { return false; }
        out.write<ubyte>({og.header, static_cast<usize>(og.header_len)});
        out.write<ubyte>({og.body, static_cast<usize>(og.body_len)});
    }

    i32 readOffset {0};
    for (;;) {
        i32 const read {std::min(1024, static_cast<i32>(samples.size()) - readOffset)};
        if (read <= 0) {
            break;
        }
        i32 const  readFrames {read / info.Channels};
        f32**      buffer {vorbis_analysis_buffer(&vd, readFrames)};
        auto const readBuffer {samples.subspan(readOffset, read)};
        readOffset += read;

        // uninterleave samples
        for (i32 i {0}; i < readFrames; i++) {
            for (i32 j {0}; j < info.Channels; ++j) {
                buffer[j][i] = readBuffer[i * info.Channels + j];
            }
        }

        vorbis_analysis_wrote(&vd, readFrames);
        flush(out, os, og, op, vd, vb);
    }

    vorbis_analysis_wrote(&vd, 0);
    flush(out, os, og, op, vd, vb);

    ogg_stream_clear(&os);
    vorbis_block_clear(&vb);
    vorbis_dsp_clear(&vd);
    vorbis_comment_clear(&vc);
    vorbis_info_clear(&vi);

    return true;
}

void vorbis_encoder::flush(ostream& out, ogg_stream_state& os, ogg_page& og, ogg_packet& op, vorbis_dsp_state& vd, vorbis_block& vb) const
{
    while (vorbis_analysis_blockout(&vd, &vb) == 1) {
        vorbis_analysis(&vb, nullptr);
        vorbis_bitrate_addblock(&vb);

        while (vorbis_bitrate_flushpacket(&vd, &op)) {
            ogg_stream_packetin(&os, &op);

            for (;;) {
                if (ogg_stream_pageout(&os, &og) == 0) {
                    break;
                }
                out.write<ubyte>({og.header, static_cast<usize>(og.header_len)});
                out.write<ubyte>({og.body, static_cast<usize>(og.body_len)});
                if (ogg_page_eos(&og)) {
                    break;
                }
            }
        }
    }
}
}

#endif
