// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "ImageCodec_png.hpp"

#include <cstring>

namespace tcob::gfx::detail {

void png_decoder::get_image_data_non_interlaced_G1(std::span<u8 const> pix)
{
    _curLine[_pixel.X / 8] = pix[0];
    filter(_pixel.X / 8, _pixel.Y, 1);

    for (i32 i {0}; i < 8 && _pixel.X < _ihdr.Width; i++) {
        u8 const c {static_cast<u8>(png::get_bits(_curLine[_pixel.X / 8], 7 - i, 1) * 255)};
        for (i32 j {0}; j < 3; ++j) { *_dataIt++ = c; }
        *_dataIt++ = _trns && _trns->is_gray_transparent(c) ? 0 : 255;

        ++_pixel.X;
    }

    if (_pixel.X >= _ihdr.Width) {
        next_line_non_interlaced();
    }
}

void png_decoder::get_image_data_non_interlaced_G2(std::span<u8 const> pix)
{
    _curLine[_pixel.X / 4] = pix[0];
    filter(_pixel.X / 4, _pixel.Y, 1);

    for (i32 i {0}; i < 8 && _pixel.X < _ihdr.Width; i += 2) {
        u8 const c {static_cast<u8>(png::get_bits(_curLine[_pixel.X / 4], 6 - i, 2) / 3.0f * 255)};
        for (i32 j {0}; j < 3; ++j) { *_dataIt++ = c; }
        *_dataIt++ = _trns && _trns->is_gray_transparent(c) ? 0 : 255;

        ++_pixel.X;
    }

    if (_pixel.X >= _ihdr.Width) {
        next_line_non_interlaced();
    }
}

void png_decoder::get_image_data_non_interlaced_G4(std::span<u8 const> pix)
{
    _curLine[_pixel.X / 2] = pix[0];
    filter(_pixel.X / 2, _pixel.Y, 1);

    for (i32 i {0}; i < 8 && _pixel.X < _ihdr.Width; i += 4) {
        u8 const c {static_cast<u8>(png::get_bits(_curLine[_pixel.X / 2], 4 - i, 4) / 15.0f * 255)};
        for (i32 j {0}; j < 3; ++j) { *_dataIt++ = c; }
        *_dataIt++ = _trns && _trns->is_gray_transparent(c) ? 0 : 255;

        ++_pixel.X;
    }

    if (_pixel.X >= _ihdr.Width) {
        next_line_non_interlaced();
    }
}

void png_decoder::get_image_data_non_interlaced_G8(std::span<u8 const> pix)
{
    _curLine[_pixel.X] = pix[0];
    filter(_pixel.X, _pixel.Y, 1);

    u8 const c {_curLine[_pixel.X]};
    for (i32 j {0}; j < 3; ++j) { *_dataIt++ = c; }
    *_dataIt++ = _trns && _trns->is_gray_transparent(c) ? 0 : 255;

    ++_pixel.X;

    if (_pixel.X >= _ihdr.Width) {
        next_line_non_interlaced();
    }
}

void png_decoder::get_image_data_non_interlaced_G16(std::span<u8 const> pix)
{
    memcpy(_curLine.data() + _lineIndex, pix.data(), pix.size());
    filter(_pixel.X, _pixel.Y, 2);

    u8 const c {_curLine[_lineIndex]};
    for (i32 j {0}; j < 3; ++j) { *_dataIt++ = c; }
    *_dataIt++ = _trns && _trns->is_gray_transparent(c) ? 0 : 255;

    _lineIndex += 2;
    ++_pixel.X;

    if (_pixel.X >= _ihdr.Width) {
        next_line_non_interlaced();
    }
}

void png_decoder::get_image_data_non_interlaced_GA8(std::span<u8 const> pix)
{
    memcpy(_curLine.data() + _lineIndex, pix.data(), pix.size());
    filter(_pixel.X, _pixel.Y, 2);

    *_dataIt = *(_dataIt + 1) = *(_dataIt + 2) = _curLine[_lineIndex++];
    *(_dataIt + 3)                             = _curLine[_lineIndex++];

    _dataIt += 4;
    ++_pixel.X;

    if (_pixel.X >= _ihdr.Width) {
        next_line_non_interlaced();
    }
}

void png_decoder::get_image_data_non_interlaced_GA16(std::span<u8 const> pix)
{
    memcpy(_curLine.data() + _lineIndex, pix.data(), pix.size());
    filter(_pixel.X, _pixel.Y, 4);

    *_dataIt = *(_dataIt + 1) = *(_dataIt + 2) = _curLine[_lineIndex];
    *(_dataIt + 3)                             = _curLine[_lineIndex + 2];

    _dataIt += 4;
    _lineIndex += 4;
    ++_pixel.X;

    if (_pixel.X >= _ihdr.Width) {
        next_line_non_interlaced();
    }
}

void png_decoder::get_image_data_non_interlaced_I1(std::span<u8 const> pix)
{
    _curLine[_pixel.X / 8] = pix[0];
    filter(_pixel.X / 8, _pixel.Y, 1);

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

    if (_pixel.X >= _ihdr.Width) {
        next_line_non_interlaced();
    }
}

void png_decoder::get_image_data_non_interlaced_I2(std::span<u8 const> pix)
{
    _curLine[_pixel.X / 4] = pix[0];
    filter(_pixel.X / 4, _pixel.Y, 1);

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

    if (_pixel.X >= _ihdr.Width) {
        next_line_non_interlaced();
    }
}

void png_decoder::get_image_data_non_interlaced_I4(std::span<u8 const> pix)
{
    _curLine[_pixel.X / 2] = pix[0];
    filter(_pixel.X / 2, _pixel.Y, 1);

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

    if (_pixel.X >= _ihdr.Width) {
        next_line_non_interlaced();
    }
}

void png_decoder::get_image_data_non_interlaced_I8(std::span<u8 const> pix)
{
    _curLine[_pixel.X] = pix[0];
    filter(_pixel.X, _pixel.Y, 1);

    u8 const idx {_curLine[_pixel.X]};
    assert(_plte && _plte->Entries.size() > idx);

    auto const color {_plte->Entries[idx]};
    *_dataIt++ = color.R;
    *_dataIt++ = color.G;
    *_dataIt++ = color.B;
    *_dataIt++ = color.A;

    ++_pixel.X;

    if (_pixel.X >= _ihdr.Width) {
        next_line_non_interlaced();
    }
}

void png_decoder::get_image_data_non_interlaced_TC8(std::span<u8 const> pix)
{
    memcpy(_curLine.data() + _lineIndex, pix.data(), pix.size());
    filter(_pixel.X, _pixel.Y, 3);

    u8 const r {_curLine[_lineIndex++]};
    u8 const g {_curLine[_lineIndex++]};
    u8 const b {_curLine[_lineIndex++]};

    *_dataIt++ = r;
    *_dataIt++ = g;
    *_dataIt++ = b;
    *_dataIt++ = _trns && _trns->is_rgb_transparent(r, g, b) ? 0 : 255;

    ++_pixel.X;

    if (_pixel.X >= _ihdr.Width) {
        next_line_non_interlaced();
    }
}

void png_decoder::get_image_data_non_interlaced_TC16(std::span<u8 const> pix)
{
    memcpy(_curLine.data() + _lineIndex, pix.data(), pix.size());
    filter(_pixel.X, _pixel.Y, 6);

    u8 const r {_curLine[_lineIndex]};
    u8 const g {_curLine[_lineIndex + 2]};
    u8 const b {_curLine[_lineIndex + 4]};

    *_dataIt++ = r;
    *_dataIt++ = g;
    *_dataIt++ = b;
    *_dataIt++ = _trns && _trns->is_rgb_transparent(r, g, b) ? 0 : 255;

    _lineIndex += 6;
    ++_pixel.X;

    if (_pixel.X >= _ihdr.Width) {
        next_line_non_interlaced();
    }
}

void png_decoder::get_image_data_non_interlaced_TCA8(std::span<u8 const> pix)
{
    memcpy(_curLine.data() + _lineIndex, pix.data(), pix.size());
    filter(_pixel.X, _pixel.Y, 4);

    for (i32 i {0}; i < 4; ++i) {
        *_dataIt++ = _curLine[_lineIndex++];
    }

    ++_pixel.X;

    if (_pixel.X >= _ihdr.Width) {
        next_line_non_interlaced();
    }
}

void png_decoder::get_image_data_non_interlaced_TCA16(std::span<u8 const> pix)
{
    memcpy(_curLine.data() + _lineIndex, pix.data(), pix.size());
    filter(_pixel.X, _pixel.Y, 8);

    for (i32 i {0}; i < 4; ++i) {
        *_dataIt++ = _curLine[_lineIndex];
        _lineIndex += 2;
    }

    ++_pixel.X;

    if (_pixel.X >= _ihdr.Width) {
        next_line_non_interlaced();
    }
}

}
