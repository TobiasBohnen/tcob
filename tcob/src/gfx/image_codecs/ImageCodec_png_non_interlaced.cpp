// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "ImageCodec_png.hpp"

#include <algorithm>

#include "tcob/core/Common.hpp"

namespace tcob::gfx::detail {

void png_decoder::non_interlaced_G1(i32 width, i32 /* height */)
{
    for (i32 i {0}; i < 8 && _pixel.X < width; i++) {
        u8 const c {static_cast<u8>(helper::get_bits(*_curLineIt, 7 - i, 1) * 255)};

        std::fill_n(_dataIt, 3, c);
        *(_dataIt + 3) = _trns && _trns->is_gray_transparent(c) ? 0 : 255;

        _dataIt += 4;
        ++_pixel.X;
    }

    if (_pixel.X >= width) { next_line_non_interlaced(); }
}

void png_decoder::non_interlaced_G2(i32 width, i32 /* height */)
{
    for (i32 i {0}; i < 8 && _pixel.X < width; i += 2) {
        u8 const c {static_cast<u8>(helper::get_bits(*_curLineIt, 6 - i, 2) / 3.0f * 255)};

        std::fill_n(_dataIt, 3, c);
        *(_dataIt + 3) = _trns && _trns->is_gray_transparent(c) ? 0 : 255;

        _dataIt += 4;
        ++_pixel.X;
    }

    if (_pixel.X >= width) { next_line_non_interlaced(); }
}

void png_decoder::non_interlaced_G4(i32 width, i32 /* height */)
{
    for (i32 i {0}; i < 8 && _pixel.X < width; i += 4) {
        u8 const c {static_cast<u8>(helper::get_bits(*_curLineIt, 4 - i, 4) / 15.0f * 255)};

        std::fill_n(_dataIt, 3, c);
        *(_dataIt + 3) = _trns && _trns->is_gray_transparent(c) ? 0 : 255;

        _dataIt += 4;
        ++_pixel.X;
    }

    if (_pixel.X >= width) { next_line_non_interlaced(); }
}

void png_decoder::non_interlaced_G8_16(i32 width, i32 /* height */)
{
    u8 const c {*_curLineIt};
    std::fill_n(_dataIt, 3, c);
    *(_dataIt + 3) = _trns && _trns->is_gray_transparent(c) ? 0 : 255;

    _dataIt += 4;

    if (++_pixel.X >= width) { next_line_non_interlaced(); }
}

void png_decoder::non_interlaced_GA8_16(i32 width, i32 /* height */)
{
    std::fill_n(_dataIt, 3, *_curLineIt);
    *(_dataIt + 3) = *(_curLineIt + (_pixelSize / 2));

    _dataIt += 4;

    if (++_pixel.X >= width) { next_line_non_interlaced(); }
}

void png_decoder::non_interlaced_I1(i32 width, i32 /* height */)
{
    for (i32 i {0}; i < 8 && _pixel.X < width; i++) {
        u8 const idx {static_cast<u8>(helper::get_bits(*_curLineIt, 7 - i, 1))};

        auto const color {_plte->Entries.at(idx)};
        *_dataIt++ = color.R;
        *_dataIt++ = color.G;
        *_dataIt++ = color.B;
        *_dataIt++ = color.A;

        ++_pixel.X;
    }

    if (_pixel.X >= width) { next_line_non_interlaced(); }
}

void png_decoder::non_interlaced_I2(i32 width, i32 /* height */)
{
    for (i32 i {0}; i < 8 && _pixel.X < width; i += 2) {
        u8 const idx {static_cast<u8>(helper::get_bits(*_curLineIt, 6 - i, 2))};

        auto const color {_plte->Entries.at(idx)};
        *_dataIt++ = color.R;
        *_dataIt++ = color.G;
        *_dataIt++ = color.B;
        *_dataIt++ = color.A;

        ++_pixel.X;
    }

    if (_pixel.X >= width) { next_line_non_interlaced(); }
}

void png_decoder::non_interlaced_I4(i32 width, i32 /* height */)
{
    for (i32 i {0}; i < 8 && _pixel.X < width; i += 4) {
        u8 const idx {static_cast<u8>(helper::get_bits(*_curLineIt, 4 - i, 4))};

        auto const color {_plte->Entries.at(idx)};
        *_dataIt++ = color.R;
        *_dataIt++ = color.G;
        *_dataIt++ = color.B;
        *_dataIt++ = color.A;

        ++_pixel.X;
    }

    if (_pixel.X >= width) { next_line_non_interlaced(); }
}

void png_decoder::non_interlaced_I8(i32 width, i32 /* height */)
{
    u8 const idx {*_curLineIt};

    auto const color {_plte->Entries.at(idx)};
    *_dataIt++ = color.R;
    *_dataIt++ = color.G;
    *_dataIt++ = color.B;
    *_dataIt++ = color.A;

    if (++_pixel.X >= width) { next_line_non_interlaced(); }
}

void png_decoder::non_interlaced_TC8_16(i32 width, i32 /* height */)
{
    u8 const r {*_curLineIt};
    u8 const g {*(_curLineIt + (_pixelSize / 3))};
    u8 const b {*(_curLineIt + (_pixelSize / 3 * 2))};
    u8 const a {_trns && _trns->is_rgb_transparent(r, g, b) ? u8 {0} : u8 {255}};
    *_dataIt++ = r;
    *_dataIt++ = g;
    *_dataIt++ = b;
    *_dataIt++ = a;

    if (++_pixel.X >= width) { next_line_non_interlaced(); }
}

void png_decoder::non_interlaced_TCA8_16(i32 width, i32 /* height */)
{
    for (i32 i {0}; i < 4; ++i) { *_dataIt++ = *(_curLineIt + (i * _pixelSize / 4)); }
    if (++_pixel.X >= width) { next_line_non_interlaced(); }
}

}
