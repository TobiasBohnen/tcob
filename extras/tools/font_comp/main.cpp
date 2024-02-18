// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "../shared/argparse.hpp"
#include <iostream>
#include <tcob/tcob.hpp>

using namespace tcob;
namespace io = tcob::io;

auto inline print_error(std::string const& err) -> int
{
    std::cout << err;
    return 1;
}

template <typename T>
void print_element(T t, int const& width)
{
    std::cout << std::left << std::setw(width) << std::setfill(' ') << t;
}

auto main(int argc, char* argv[]) -> int
{
    argparse::ArgumentParser program("fontcomp");
    program.add_argument("input");
    program.add_argument("output");
    program.add_argument("-s", "--size")
        .default_value(16)
        .scan<'i', i32>();
    program.add_argument("-se", "--single-engine")
        .choices("freetype", "stbtt", "libschrift");
    auto pl {platform::HeadlessInit(argv[0])};

    try {
        program.parse_args(argc, argv);
    } catch (std::exception const& err) {
        std::cout << err.what() << '\n';
        std::cout << program;
        return 1;
    }

    std::string const src {program.get("input")};
    std::string const dst {program.get("output")};
    i32 const         size {program.get<i32>("--size")};
    bool              singleMode {program.is_used("--single-engine")};

    if (!io::is_file(src)) {
        return print_error("file not found: " + src);
    }

    auto fs {io::ifstream::Open(src)};
    auto fontData {fs->read_all<ubyte>()};

    auto& ttfFactory {locate_service<gfx::truetype_font_engine::factory>()};

    auto engFT {ttfFactory.create("FREETYPE")};
    auto engSTB {ttfFactory.create("STBTT")};
    auto engLS {ttfFactory.create("LIBSCHRIFT")};

    auto infoFT {engFT->load_data(fontData, size)};
    if (!infoFT) {
        return print_error("FreeType failed to load: " + src);
    }

    auto infoSTB {engSTB->load_data(fontData, size)};
    if (!infoSTB) {
        return print_error("STB_truetype failed to load: " + src);
    }

    auto infoLS {engLS->load_data(fontData, size)};
    if (!infoLS) {
        return print_error("libschrift failed to load: " + src);
    }

    data::config::object obj;

    if (singleMode) {
        auto writeToObj {[&](tcob::gfx::truetype_font_engine* engine, gfx::font::info const& info) {
            obj["Font"]["File"]       = src;
            obj["Font"]["Size"]       = size;
            obj["Font"]["Engine"]     = engine->get_name();
            obj["Info"]["Ascender"]   = info.Ascender;
            obj["Info"]["Descender"]  = info.Descender;
            obj["Info"]["LineHeight"] = info.LineHeight;

            string const chars {"0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ"};

            for (char i : chars) {
                auto const glyph {engine->get_glyph(i)};

                auto const key {"\"" + std::string {i} + "\""};
                obj["Glyphs"][key]["AdvanceX"] = glyph.Glyph.AdvanceX;
                obj["Glyphs"][key]["Offset"]   = glyph.Glyph.Offset;
                obj["Glyphs"][key]["Size"]     = glyph.Glyph.Size;
            }
        }};

        std::string const se {program.get("--single-engine")};

        if (se == "freetype") {
            writeToObj(engFT.get(), *infoFT);
        } else if (se == "stbtt") {
            writeToObj(engSTB.get(), *infoSTB);
        } else if (se == "libschrift") {
            writeToObj(engLS.get(), *infoLS);
        }
    } else {
        auto writeToObj {[&](tcob::gfx::truetype_font_engine* engine, gfx::font::info const& info) {
            auto const engineName {engine->get_name()};

            obj["Font"]["File"]                   = src;
            obj["Font"]["Size"]                   = size;
            obj["Info"]["Ascender"][engineName]   = info.Ascender;
            obj["Info"]["Descender"][engineName]  = info.Descender;
            obj["Info"]["LineHeight"][engineName] = info.LineHeight;

            string const chars {"0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ"};

            for (char i : chars) {
                auto const glyph {engine->get_glyph(i)};

                auto const key {"\"" + std::string {i} + "\""};
                obj["Glyphs"][key]["AdvanceX"][engineName] = glyph.Glyph.AdvanceX;
                obj["Glyphs"][key]["Offset"][engineName]   = glyph.Glyph.Offset;
                obj["Glyphs"][key]["Size"][engineName]     = glyph.Glyph.Size;
            }
        }};

        writeToObj(engFT.get(), *infoFT);
        writeToObj(engSTB.get(), *infoSTB);
        writeToObj(engLS.get(), *infoLS);
    }

    obj.save(dst);

    return 0;
}