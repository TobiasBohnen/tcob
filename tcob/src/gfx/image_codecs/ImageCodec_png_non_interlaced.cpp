// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "ImageCodec_png.hpp"

namespace tcob::gfx::detail {

void png_decoder::non_interlaced_G1()
{
    for (i32 i {0}; i < 8 && _pixel.X < _ihdr.Width; i++) {
        u8 const c {static_cast<u8>(png::get_bits(_curLine[_pixel.X / 8], 7 - i, 1) * 255)};

        std::fill_n(_dataIt, 3, c);
        *(_dataIt + 3) = _trns && _trns->is_gray_transparent(c) ? 0 : 255;

        _dataIt += 4;
        ++_pixel.X;
    }

    if (_pixel.X >= _ihdr.Width) { next_line_non_interlaced(); }
}

void png_decoder::non_interlaced_G2()
{
    for (i32 i {0}; i < 8 && _pixel.X < _ihdr.Width; i += 2) {
        u8 const c {static_cast<u8>(png::get_bits(_curLine[_pixel.X / 4], 6 - i, 2) / 3.0f * 255)};

        std::fill_n(_dataIt, 3, c);
        *(_dataIt + 3) = _trns && _trns->is_gray_transparent(c) ? 0 : 255;

        _dataIt += 4;
        ++_pixel.X;
    }

    if (_pixel.X >= _ihdr.Width) { next_line_non_interlaced(); }
}

void png_decoder::non_interlaced_G4()
{
    for (i32 i {0}; i < 8 && _pixel.X < _ihdr.Width; i += 4) {
        u8 const c {static_cast<u8>(png::get_bits(_curLine[_pixel.X / 2], 4 - i, 4) / 15.0f * 255)};

        std::fill_n(_dataIt, 3, c);
        *(_dataIt + 3) = _trns && _trns->is_gray_transparent(c) ? 0 : 255;

        _dataIt += 4;
        ++_pixel.X;
    }

    if (_pixel.X >= _ihdr.Width) { next_line_non_interlaced(); }
}

void png_decoder::non_interlaced_G8()
{
    u8 const c {_curLine[_pixel.X]};
    std::fill_n(_dataIt, 3, c);
    *(_dataIt + 3) = _trns && _trns->is_gray_transparent(c) ? 0 : 255;

    _dataIt += 4;

    if (++_pixel.X >= _ihdr.Width) { next_line_non_interlaced(); }
}

void png_decoder::non_interlaced_G16()
{
    u8 const c {*_curLineIt};
    std::fill_n(_dataIt, 3, c);
    *(_dataIt + 3) = _trns && _trns->is_gray_transparent(c) ? 0 : 255;

    _dataIt += 4;

    if (++_pixel.X >= _ihdr.Width) { next_line_non_interlaced(); }
}

void png_decoder::non_interlaced_GA8()
{
    std::fill_n(_dataIt, 3, *_curLineIt);
    *(_dataIt + 3) = *(_curLineIt + 1);

    _dataIt += 4;

    if (++_pixel.X >= _ihdr.Width) { next_line_non_interlaced(); }
}

void png_decoder::non_interlaced_GA16()
{
    std::fill_n(_dataIt, 3, *_curLineIt);
    *(_dataIt + 3) = *(_curLineIt + 2);

    _dataIt += 4;

    if (++_pixel.X >= _ihdr.Width) { next_line_non_interlaced(); }
}

void png_decoder::non_interlaced_I1()
{
    for (i32 i {0}; i < 8 && _pixel.X < _ihdr.Width; i++) {
        u8 const idx {png::get_bits(_curLine[_pixel.X / 8], 7 - i, 1)};
        assert(_plte && _plte->Entries.size() > idx);

        auto const color {_plte->Entries[idx]};
        *_dataIt++ = color.R;
        *_dataIt++ = color.G;
        *_dataIt++ = color.B;
        *_dataIt++ = color.A;

        ++_pixel.X;
    }

    if (_pixel.X >= _ihdr.Width) { next_line_non_interlaced(); }
}

void png_decoder::non_interlaced_I2()
{
    for (i32 i {0}; i < 8 && _pixel.X < _ihdr.Width; i += 2) {
        u8 const idx {png::get_bits(_curLine[_pixel.X / 4], 6 - i, 2)};
        assert(_plte && _plte->Entries.size() > idx);

        auto const color {_plte->Entries[idx]};
        *_dataIt++ = color.R;
        *_dataIt++ = color.G;
        *_dataIt++ = color.B;
        *_dataIt++ = color.A;

        ++_pixel.X;
    }

    if (_pixel.X >= _ihdr.Width) { next_line_non_interlaced(); }
}

void png_decoder::non_interlaced_I4()
{
    for (i32 i {0}; i < 8 && _pixel.X < _ihdr.Width; i += 4) {
        u8 const idx {png::get_bits(_curLine[_pixel.X / 2], 4 - i, 4)};
        assert(_plte && _plte->Entries.size() > idx);

        auto const color {_plte->Entries[idx]};
        *_dataIt++ = color.R;
        *_dataIt++ = color.G;
        *_dataIt++ = color.B;
        *_dataIt++ = color.A;

        ++_pixel.X;
    }

    if (_pixel.X >= _ihdr.Width) { next_line_non_interlaced(); }
}

void png_decoder::non_interlaced_I8()
{
    u8 const idx {*_curLineIt};
    assert(_plte && _plte->Entries.size() > idx);

    auto const color {_plte->Entries[idx]};
    *_dataIt++ = color.R;
    *_dataIt++ = color.G;
    *_dataIt++ = color.B;
    *_dataIt++ = color.A;

    if (++_pixel.X >= _ihdr.Width) { next_line_non_interlaced(); }
}

void png_decoder::non_interlaced_TC8()
{
    u8 const r {*_curLineIt};
    u8 const g {*(_curLineIt + 1)};
    u8 const b {*(_curLineIt + 2)};

    *_dataIt++ = r;
    *_dataIt++ = g;
    *_dataIt++ = b;
    *_dataIt++ = _trns && _trns->is_rgb_transparent(r, g, b) ? 0 : 255;

    if (++_pixel.X >= _ihdr.Width) { next_line_non_interlaced(); }
}

void png_decoder::non_interlaced_TC16()
{
    u8 const r {*_curLineIt};
    u8 const g {*(_curLineIt + 2)};
    u8 const b {*(_curLineIt + 4)};

    *_dataIt++ = r;
    *_dataIt++ = g;
    *_dataIt++ = b;
    *_dataIt++ = _trns && _trns->is_rgb_transparent(r, g, b) ? 0 : 255;

    if (++_pixel.X >= _ihdr.Width) { next_line_non_interlaced(); }
}

void png_decoder::non_interlaced_TCA8()
{
    for (i32 i {0}; i < 4; ++i) { *_dataIt++ = *(_curLineIt + i); }
    if (++_pixel.X >= _ihdr.Width) { next_line_non_interlaced(); }
}

void png_decoder::non_interlaced_TCA16()
{
    for (i32 i {0}; i < 4; ++i) {
        *_dataIt++ = *(_curLineIt + (i * 2));
    }

    if (++_pixel.X >= _ihdr.Width) { next_line_non_interlaced(); }
}

}
