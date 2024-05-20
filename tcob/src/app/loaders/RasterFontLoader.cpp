// Copyright (c) 2023 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "RasterFontLoader.hpp"

#include "tcob/core/io/FileStream.hpp"
#include "tcob/data/Config.hpp"
#include "tcob/data/ConfigTypes.hpp"

namespace tcob::detail {

using namespace tcob::data::config;

auto ini_raster_font_loader::load(gfx::raster_font& font, path const& file, string const& textureFolder) -> std::optional<gfx::font::info>
{
    object config;
    if (config.load(file) != load_status::Ok) {
        return std::nullopt;
    }

    size_i const fontTextureSize {config["info"]["texture_size"].as<size_i>()};

    // glyphs
    for (auto& [_, s] : config["glyphs"].as<object>()) {
        auto                g {s.get<object>().value()};
        gfx::rendered_glyph glyph;
        glyph.Size      = g["size"].as<size_i>();
        glyph.Offset    = g["offset"].as<point_f>();
        glyph.AdvanceX  = g["advance_x"].as<f32>();
        glyph.TexRegion = g["tex_region"].as<gfx::texture_region>();
        glyph.TexRegion.UVRect.X /= fontTextureSize.Width;
        glyph.TexRegion.UVRect.Width /= fontTextureSize.Width;
        glyph.TexRegion.UVRect.Y /= fontTextureSize.Height;
        glyph.TexRegion.UVRect.Height /= fontTextureSize.Height;
        font.add_glyph(g["id"].as<u32>(), glyph);
    }

    for (auto& [_, s] : config["kerning_pairs"].as<object>()) {
        auto k {s.get<object>().value()};
        font.add_kerning_pair(k["first"].as<u32>(), k["second"].as<u32>(), k["amount"].as<i16>());
    }

    for (auto& [_, s] : config["pages"].as<object>()) {
        gfx::image img;
        if (img.load(textureFolder + "/" + s.get<string>().value()) == load_status::Ok) {
            assert(img.get_info().Size == fontTextureSize);
            font.add_image(img);
        }
    }

    return gfx::font::info {.Ascender   = config["info"]["ascender"].as<f32>(),
                            .Descender  = config["info"]["ascender"].as<f32>(),
                            .LineHeight = config["info"]["line_height"].as<f32>()};
}

////////////////////////////////////////////////////////////

auto fnt_raster_font_loader::load(gfx::raster_font& font, path const& file, string const& textureFolder) -> std::optional<gfx::font::info>
{
    using seek_dir = io::seek_dir;

    io::ifstream fs {file};
    auto const   magic {fs.read<std::array<byte, 3>>()};
    byte const   version {fs.read<byte>()};

    if (magic != std::array<byte, 3> {'B', 'M', 'F'} || version != 3) {
        return std::nullopt;
    }

    u16                 pages {0};
    std::vector<string> pageNames {};
    size_i              fontTextureSize {size_u::Zero};
    u16                 lineHeight {0};
    u16                 base {0};

    while (!fs.is_eof()) {
        byte const blockType {fs.read<byte>()};
        u32 const  blockSize {fs.read<u32>()};

        switch (blockType) {

        case 1: {
            // info block (ignored)
            fs.seek(blockSize, seek_dir::Current);
        } break;

        case 2: {
            // common block
            lineHeight = fs.read<u16>();             // lineHeight 2 uint 0
            base       = fs.read<u16>();             // base       2 uint 2

            fontTextureSize.Width  = fs.read<u16>(); // scaleW 2 uint 4
            fontTextureSize.Height = fs.read<u16>(); // scaleH 2 uint 6
            pages                  = fs.read<u16>(); // pages 2 uint 8

            // bitField  1 bits 10 (ignored) bits 0-6: reserved, bit 7: packed
            // alphaChnl 1 uint 11 (ignored)
            // redChnl   1 uint 12 (ignored)
            // greenChnl 1 uint 13 (ignored)
            // blueChnl  1 uint 14 (ignored)
            fs.seek(5, seek_dir::Current);
        } break;

        case 3: {
            if (pages > 0) {
                u32 stringSize {blockSize / pages};
                for (u16 i {0}; i < pages; ++i) {
                    pageNames.push_back(fs.read_string(stringSize - 1));
                    fs.read<byte>(); // null terminator
                }
            }
        } break;

        case 4: {
            // chars block
            u32 const charCount {blockSize / 20};
            for (u32 i {0}; i < charCount; ++i) {
                u32 const   id {fs.read<u32>()};       // id       4 uint 0+c*20     These fields are repeated until all characters have been described
                u16 const   x {fs.read<u16>()};        // x        2 uint 4+c*20
                u16 const   y {fs.read<u16>()};        // y        2 uint 6+c*20
                u16 const   width {fs.read<u16>()};    // width    2 uint 8+c*20
                u16 const   height {fs.read<u16>()};   // height   2 uint 10+c*20
                i16 const   xoffset {fs.read<i16>()};  // xoffset  2 int  12+c*20
                i16 const   yoffset {fs.read<i16>()};  // yoffset  2 int  14+c*20
                i16 const   xadvance {fs.read<i16>()}; // xadvance 2 int  16+c*20
                ubyte const page {fs.read<ubyte>()};
                // page 1 uint 18+c*20 (ignored)
                // chnl 1 uint 19+c*20 (ignored)
                fs.seek(1, seek_dir::Current);

                // create glyph
                gfx::rendered_glyph glyph;
                glyph.Size      = size_i {width, height};
                glyph.Offset    = point_f {static_cast<f32>(xoffset), static_cast<f32>(yoffset)};
                glyph.AdvanceX  = static_cast<f32>(xadvance);
                glyph.TexRegion = {rect_f {static_cast<f32>(x), static_cast<f32>(y), static_cast<f32>(width), static_cast<f32>(height)}, page};
                glyph.TexRegion.UVRect.X /= fontTextureSize.Width;
                glyph.TexRegion.UVRect.Width /= fontTextureSize.Width;
                glyph.TexRegion.UVRect.Y /= fontTextureSize.Height;
                glyph.TexRegion.UVRect.Height /= fontTextureSize.Height;
                font.add_glyph(id, glyph);
            }
        } break;

        case 5: {
            // kerning pairs block
            u32 const pairsCount {blockSize / 10};
            for (u32 i {0}; i < pairsCount; i++) {
                u32 const first {fs.read<u32>()};  // first  4 uint 0+c*10     These fields are repeated until all kerning pairs have been described
                u32 const second {fs.read<u32>()}; // second 4 uint 4+c*10
                i16 const amount {fs.read<i16>()}; // amount 2 int  8+c*6
                font.add_kerning_pair(first, second, amount);
            }

        } break;

        default: {
            // skipping unexpected block type
        } break;
        }
    }

    if (pages > 0) {
        for (u16 i {0}; i < pages; ++i) {
            gfx::image img;
            if (img.load(textureFolder + "/" + pageNames[i]) == load_status::Ok) {
                assert(img.get_info().Size == fontTextureSize);
                font.add_image(img);
            }
        }
    }

    return gfx::font::info {.Ascender   = static_cast<f32>(base),
                            .Descender  = static_cast<f32>(-(lineHeight - base)),
                            .LineHeight = static_cast<f32>(lineHeight)};
}
}
