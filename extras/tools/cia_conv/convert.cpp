// Copyright (c) 2023 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "common.hpp"

auto convert_config(std::shared_ptr<io::ifstream>& in, std::string const& src, std::string const& srcExt, std::string const& dst) -> int
{
    using namespace tcob::data::config;
    std::cout << "converting config file: " << src << " to " << dst << "\n";

    in->seek(0, io::seek_dir::Begin);

    object obj;
    if (obj.load(*in, srcExt) == load_status::Error) {
        return print_error("error loading config: " + src);
    }

    if (!obj.save(dst)) {
        return print_error("error saving config: " + dst);
    }

    std::cout << "done!\n";
    return 0;
}

auto convert_image(std::shared_ptr<io::ifstream>& in, std::string const& src, std::string const& srcExt, std::string const& dst) -> int
{
    using namespace tcob::gfx;
    std::cout << "converting image: " << src << " to " << dst << "\n";

    in->seek(0, io::seek_dir::Begin);

    image img;
    if (img.load(*in, srcExt) == load_status::Error) {
        return print_error("error loading image: " + src);
    }

    auto const& info {img.get_info()};
    std::cout << std::format("source info: BPP: {}, Width: {}, Height: {} \n",
                             (info.Format == image::format::RGBA ? 4 : 3), info.Size.Width, info.Size.Width);

    if (!img.save(dst)) {
        return print_error("error saving image: " + dst);
    }

    std::cout << "done!\n";
    return 0;
}

auto convert_audio(std::shared_ptr<io::ifstream>& in, std::string const& src, std::string const& srcExt, std::string const& dst, std::string const& ctx) -> int
{
    using namespace tcob::audio;
    std::cout << "converting audio: " << src << " to " << dst << "\n";

    in->seek(0, io::seek_dir::Begin);

    std::any context {0};

#if defined(TCOB_ENABLE_ADDON_AUDIO_TINYSOUNDFONT)
    assets::manual_asset_ptr<sound_font> sf;
    if (!ctx.empty()) {
        if (sf->load(ctx) != load_status::Ok) {
            return print_error("error loading sound font: " + ctx);
        }
        context = assets::asset_ptr<sound_font> {sf};
    }
#endif

    buffer bfr;
    if (bfr.load(in, srcExt, context) == load_status::Error) {
        return print_error("error loading image: " + src);
    }

    auto const& info {bfr.get_info()};
    std::cout << std::format("source info: Channels: {}, Frames: {}, Sample Rate: {} \n",
                             info.Channels, info.FrameCount, info.SampleRate);

    if (!bfr.save(dst)) {
        return print_error("error saving image: " + dst);
    }

    std::cout << "done!\n";
    return 0;
}

auto static convert_rfx(std::shared_ptr<io::ifstream>& in, std::string const& src, std::string const& srcExt, std::string const& dst) -> int
{
    std::cout << "converting rfx: " << src << " to " << dst << "\n";

    struct RFXFile {
        std::array<byte, 4> signature;
        u16                 version;
        u16                 length;
        i32                 randSeed;
        i32                 waveTypeValue;
        f32                 attackTimeValue;
        f32                 sustainTimeValue;
        f32                 sustainPunchValue;
        f32                 decayTimeValue;
        f32                 startFrequencyValue;
        f32                 minFrequencyValue;
        f32                 slideValue;
        f32                 deltaSlideValue;
        f32                 vibratoDepthValue;
        f32                 vibratoSpeedValue;
        f32                 changeAmountValue;
        f32                 changeSpeedValue;
        f32                 squareDutyValue;
        f32                 dutySweepValue;
        f32                 repeatSpeedValue;
        f32                 phaserOffsetValue;
        f32                 phaserSweepValue;
        f32                 lpfCutoffValue;
        f32                 lpfCutoffSweepValue;
        f32                 lpfResonanceValue;
        f32                 hpfCutoffValue;
        f32                 hpfCutoffSweepValue;
    };

    RFXFile const rfx {in->read<RFXFile>()};

    if ((rfx.signature != std::array<byte, 4> {'r', 'F', 'X', ' '}) || rfx.version != 200 || rfx.length != 96) {
        return print_error("unsupported rfx file: " + src);
    }

    audio::sound_wave wave {
        .RandomSeed                = static_cast<u64>(rfx.randSeed),
        .WaveType                  = static_cast<audio::sound_wave::type>(rfx.waveTypeValue),
        .AttackTime                = rfx.attackTimeValue,
        .SustainTime               = rfx.sustainTimeValue,
        .SustainPunch              = rfx.sustainPunchValue,
        .DecayTime                 = rfx.decayTimeValue,
        .StartFrequency            = rfx.startFrequencyValue,
        .MinFrequency              = rfx.minFrequencyValue,
        .Slide                     = rfx.slideValue,
        .DeltaSlide                = rfx.deltaSlideValue,
        .VibratoDepth              = rfx.vibratoDepthValue,
        .VibratoSpeed              = rfx.vibratoSpeedValue,
        .ChangeAmount              = rfx.changeAmountValue,
        .ChangeSpeed               = rfx.changeSpeedValue,
        .SquareDuty                = rfx.squareDutyValue,
        .DutySweep                 = rfx.dutySweepValue,
        .RepeatSpeed               = rfx.repeatSpeedValue,
        .PhaserOffset              = rfx.phaserOffsetValue,
        .PhaserSweep               = rfx.phaserSweepValue,
        .LowPassFilterCutoff       = rfx.lpfCutoffValue,
        .LowPassFilterCutoffSweep  = rfx.lpfCutoffSweepValue,
        .LowPassFilterResonance    = rfx.lpfResonanceValue,
        .HighPassFilterCutoff      = rfx.hpfCutoffValue,
        .HighPassFilterCutoffSweep = rfx.hpfCutoffSweepValue};

    auto dstGroup(io::magic::get_group(io::get_extension(dst)));
    if (dstGroup == "config") {
        data::config::object obj;
        audio::sound_wave::Serialize(wave, obj["wave"]);

        if (!obj.save(dst)) {
            return print_error("error saving rfx config: " + dst);
        }

        std::cout << "done!\n";
        return 0;
    }

    if (dstGroup == "audio") {
        audio::sound_generator gen;
        if (!gen.create_buffer(wave).save(dst)) {
            return print_error("error saving rfx audio: " + dst);
        }

        std::cout << "done!\n";
        return 0;
    }

    return print_error("unsupported convert target format: " + dst);
}

auto static convert_fnt(std::shared_ptr<io::ifstream>& in, std::string const& src, std::string const& srcExt, std::string const& dst) -> int
{
    std::cout << "converting fnt: " << src << " to " << dst << "\n";

    data::config::object obj {};

    using seek_dir = io::seek_dir;

    auto const magic {in->read<std::array<byte, 3>>()};
    byte const version {in->read<byte>()};

    if (magic != std::array<byte, 3> {'B', 'M', 'F'} || version != 3) {
        return print_error("unsupported fnt file: " + src);
    }

    u16    pages {0};
    size_i fontTextureSize {size_u::Zero};
    u16    lineHeight {0};
    u16    base {0};

    while (!in->is_eof()) {
        byte const blockType {in->read<byte>()};
        u32 const  blockSize {in->read<u32>()};

        switch (blockType) {

        case 1: {
            // info block (ignored)
            in->seek(blockSize, seek_dir::Current);
        } break;

        case 2: {
            // common block
            lineHeight = in->read<u16>();             // lineHeight 2 uint 0
            base       = in->read<u16>();             // base       2 uint 2

            fontTextureSize.Width  = in->read<u16>(); // scaleW 2 uint 4
            fontTextureSize.Height = in->read<u16>(); // scaleH 2 uint 6
            pages                  = in->read<u16>(); // pages 2 uint 8

            // bitField  1 bits 10 (ignored) bits 0-6: reserved, bit 7: packed
            // alphaChnl 1 uint 11 (ignored)
            // redChnl   1 uint 12 (ignored)
            // greenChnl 1 uint 13 (ignored)
            // blueChnl  1 uint 14 (ignored)
            in->seek(5, seek_dir::Current);
        } break;

        case 3: {
            if (pages > 0) {
                u32 stringSize {blockSize / pages};
                for (u16 i {0}; i < pages; ++i) {
                    obj["pages"]["p" + std::to_string(i)] = in->read_string(stringSize - 1);
                    in->read<byte>(); // null terminator
                }
            }
        } break;

        case 4: {
            // chars block
            u32 const charCount {blockSize / 20};
            for (u32 i {0}; i < charCount; ++i) {
                u32 const   id {in->read<u32>()};       // id       4 uint 0+c*20     These fields are repeated until all characters have been described
                u16 const   x {in->read<u16>()};        // x        2 uint 4+c*20
                u16 const   y {in->read<u16>()};        // y        2 uint 6+c*20
                u16 const   width {in->read<u16>()};    // width    2 uint 8+c*20
                u16 const   height {in->read<u16>()};   // height   2 uint 10+c*20
                i16 const   xoffset {in->read<i16>()};  // xoffset  2 int  12+c*20
                i16 const   yoffset {in->read<i16>()};  // yoffset  2 int  14+c*20
                i16 const   xadvance {in->read<i16>()}; // xadvance 2 int  16+c*20
                ubyte const page {in->read<ubyte>()};
                // page 1 uint 18+c*20 (ignored)
                // chnl 1 uint 19+c*20 (ignored)
                in->seek(1, seek_dir::Current);

                // create glyph
                gfx::rendered_glyph glyph;
                glyph.Size      = size_i {width, height},
                glyph.Offset    = point_f {static_cast<f32>(xoffset), static_cast<f32>(yoffset)},
                glyph.AdvanceX  = static_cast<f32>(xadvance),
                glyph.TexRegion = {rect_f {static_cast<f32>(x), static_cast<f32>(y), static_cast<f32>(width), static_cast<f32>(height)}, page};
                glyph.TexRegion.UVRect.X /= fontTextureSize.Width;
                glyph.TexRegion.UVRect.Width /= fontTextureSize.Width;
                glyph.TexRegion.UVRect.Y /= fontTextureSize.Height;
                glyph.TexRegion.UVRect.Height /= fontTextureSize.Height;

                data::config::object gl {};
                gl["id"]         = id;
                gl["size"]       = glyph.Size;
                gl["offset"]     = glyph.Offset;
                gl["advance_x"]  = glyph.AdvanceX;
                gl["tex_region"] = glyph.TexRegion;

                obj["glyphs"]["g" + std::to_string(i)] = gl;
            }
        } break;

        case 5: {
            // kerning pairs block
            u32 const pairsCount {blockSize / 10};
            for (u32 i {0}; i < pairsCount; i++) {
                data::config::object kp {};
                kp["first"]  = in->read<u32>();
                kp["second"] = in->read<u32>();
                kp["amount"] = in->read<i16>();

                obj["kerning_pairs"]["k" + std::to_string(i)] = kp;
            }

        } break;

        default: {
            // skipping unexpected block type
        } break;
        }
    }

    obj["info"]["texture_size"] = fontTextureSize;
    obj["info"]["ascender"]     = static_cast<f32>(base);
    obj["info"]["descender"]    = static_cast<f32>(-(lineHeight - base));
    obj["info"]["line_height"]  = static_cast<f32>(base);

    if (!obj.save(dst)) {
        return print_error("error saving rfx config: " + dst);
    }

    std::cout << "done!\n";
    return 0;
}

auto convert_misc(std::shared_ptr<io::ifstream>& in, std::string const& src, std::string const& srcExt, std::string const& dst) -> int
{
    if (srcExt == ".rfx") {
        return convert_rfx(in, src, srcExt, dst);
    }
    if (srcExt == ".fnt") {
        return convert_fnt(in, src, srcExt, dst);
    }
    return print_error("unsupported file: " + src);
}