// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "ImageCodec_png.hpp"

#include <cstring>

namespace tcob::gfx::detail {

void png_decoder::get_image_data_non_interlaced_G1(std::span<u8 const> dat)
{
    _curLine[_pixel.X / 8] = dat[0];
    filter(_pixel.X / 8, _pixel.Y, 1);

    for (i32 i {0}; i < 8 && _pixel.X < _ihdr.Width; i++) {
        if (_dataIndex + 3 < std::ssize(_data)) {
            u8 const c {static_cast<u8>(png::get_bits(_curLine[_pixel.X / 8], 7 - i, 1) * 255)};
            _data[_dataIndex++] = c;
            _data[_dataIndex++] = c;
            _data[_dataIndex++] = c;
            _data[_dataIndex++] = _trns && _trns->is_gray_transparent(c) ? 0 : 255;
        }

        ++_pixel.X;
    }

    if (_pixel.X >= _ihdr.Width) {
        next_line_non_interlaced();
    }
}

void png_decoder::get_image_data_non_interlaced_G2(std::span<u8 const> dat)
{
    _curLine[_pixel.X / 4] = dat[0];
    filter(_pixel.X / 4, _pixel.Y, 1);

    for (i32 i {0}; i < 8 && _pixel.X < _ihdr.Width; i += 2) {
        if (_dataIndex + 3 < std::ssize(_data)) {
            u8 const c {static_cast<u8>(png::get_bits(_curLine[_pixel.X / 4], 6 - i, 2) / 3.0f * 255)};
            _data[_dataIndex++] = c;
            _data[_dataIndex++] = c;
            _data[_dataIndex++] = c;
            _data[_dataIndex++] = _trns && _trns->is_gray_transparent(c) ? 0 : 255;
        }

        ++_pixel.X;
    }

    if (_pixel.X >= _ihdr.Width) {
        next_line_non_interlaced();
    }
}

void png_decoder::get_image_data_non_interlaced_G4(std::span<u8 const> dat)
{
    _curLine[_pixel.X / 2] = dat[0];
    filter(_pixel.X / 2, _pixel.Y, 1);

    for (i32 i {0}; i < 8 && _pixel.X < _ihdr.Width; i += 4) {
        if (_dataIndex + 3 < std::ssize(_data)) {
            u8 const c {static_cast<u8>(png::get_bits(_curLine[_pixel.X / 2], 4 - i, 4) / 15.0f * 255)};
            _data[_dataIndex++] = c;
            _data[_dataIndex++] = c;
            _data[_dataIndex++] = c;
            _data[_dataIndex++] = _trns && _trns->is_gray_transparent(c) ? 0 : 255;
        }

        ++_pixel.X;
    }

    if (_pixel.X >= _ihdr.Width) {
        next_line_non_interlaced();
    }
}

void png_decoder::get_image_data_non_interlaced_G8(std::span<u8 const> dat)
{
    _curLine[_pixel.X] = dat[0];
    filter(_pixel.X, _pixel.Y, 1);

    if (_dataIndex + 3 < std::ssize(_data)) {
        u8 const c {_curLine[_pixel.X]};
        _data[_dataIndex++] = c;
        _data[_dataIndex++] = c;
        _data[_dataIndex++] = c;
        _data[_dataIndex++] = _trns && _trns->is_gray_transparent(c) ? 0 : 255;
    }

    ++_pixel.X;

    if (_pixel.X >= _ihdr.Width) {
        next_line_non_interlaced();
    }
}

void png_decoder::get_image_data_non_interlaced_G16(std::span<u8 const> dat)
{
    memcpy(_curLine.data() + _lineIndex, dat.data(), dat.size());
    filter(_pixel.X, _pixel.Y, 2);

    if (_dataIndex + 3 < std::ssize(_data)) {
        u8 const c {_curLine[_lineIndex]};
        _data[_dataIndex++] = c;
        _data[_dataIndex++] = c;
        _data[_dataIndex++] = c;
        _data[_dataIndex++] = _trns && _trns->is_gray_transparent(c) ? 0 : 255;
    }

    _lineIndex += 2;
    ++_pixel.X;

    if (_pixel.X >= _ihdr.Width) {
        next_line_non_interlaced();
    }
}

void png_decoder::get_image_data_non_interlaced_GA8(std::span<u8 const> dat)
{
    memcpy(_curLine.data() + _lineIndex, dat.data(), dat.size());
    filter(_pixel.X, _pixel.Y, 2);

    if (_dataIndex + 3 < std::ssize(_data)) {
        _data[_dataIndex] = _data[_dataIndex + 1] = _data[_dataIndex + 2] = _curLine[_lineIndex++];
        _data[_dataIndex + 3]                                             = _curLine[_lineIndex++];
    }

    _dataIndex += 4;
    ++_pixel.X;

    if (_pixel.X >= _ihdr.Width) {
        next_line_non_interlaced();
    }
}

void png_decoder::get_image_data_non_interlaced_GA16(std::span<u8 const> dat)
{
    memcpy(_curLine.data() + _lineIndex, dat.data(), dat.size());
    filter(_pixel.X, _pixel.Y, 4);

    if (_dataIndex + 3 < std::ssize(_data)) {
        _data[_dataIndex] = _data[_dataIndex + 1] = _data[_dataIndex + 2] = _curLine[_lineIndex];
        _data[_dataIndex + 3]                                             = _curLine[_lineIndex + 2];
    }

    _dataIndex += 4;
    _lineIndex += 4;
    ++_pixel.X;

    if (_pixel.X >= _ihdr.Width) {
        next_line_non_interlaced();
    }
}

void png_decoder::get_image_data_non_interlaced_I1(std::span<u8 const> dat)
{
    _curLine[_pixel.X / 8] = dat[0];
    filter(_pixel.X / 8, _pixel.Y, 1);

    for (i32 i {0}; i < 8 && _pixel.X < _ihdr.Width; i++) {
        u8 const idx {png::get_bits(_curLine[_pixel.X / 8], 7 - i, 1)};
        assert(_plte && _plte->Entries.size() > idx);

        if (_dataIndex + 3 < std::ssize(_data)) {
            auto const color {_plte->Entries[idx]};
            _data[_dataIndex++] = color.R;
            _data[_dataIndex++] = color.G;
            _data[_dataIndex++] = color.B;
            _data[_dataIndex++] = color.A;
        }

        ++_pixel.X;
    }

    if (_pixel.X >= _ihdr.Width) {
        next_line_non_interlaced();
    }
}

void png_decoder::get_image_data_non_interlaced_I2(std::span<u8 const> dat)
{
    _curLine[_pixel.X / 4] = dat[0];
    filter(_pixel.X / 4, _pixel.Y, 1);

    for (i32 i {0}; i < 8 && _pixel.X < _ihdr.Width; i += 2) {
        u8 const idx {png::get_bits(_curLine[_pixel.X / 4], 6 - i, 2)};
        assert(_plte && _plte->Entries.size() > idx);

        if (_dataIndex + 3 < std::ssize(_data)) {
            auto const color {_plte->Entries[idx]};
            _data[_dataIndex++] = color.R;
            _data[_dataIndex++] = color.G;
            _data[_dataIndex++] = color.B;
            _data[_dataIndex++] = color.A;
        }

        ++_pixel.X;
    }

    if (_pixel.X >= _ihdr.Width) {
        next_line_non_interlaced();
    }
}

void png_decoder::get_image_data_non_interlaced_I4(std::span<u8 const> dat)
{
    _curLine[_pixel.X / 2] = dat[0];
    filter(_pixel.X / 2, _pixel.Y, 1);

    for (i32 i {0}; i < 8 && _pixel.X < _ihdr.Width; i += 4) {
        u8 const idx {png::get_bits(_curLine[_pixel.X / 2], 4 - i, 4)};
        assert(_plte && _plte->Entries.size() > idx);

        if (_dataIndex + 3 < std::ssize(_data)) {
            auto const color {_plte->Entries[idx]};
            _data[_dataIndex++] = color.R;
            _data[_dataIndex++] = color.G;
            _data[_dataIndex++] = color.B;
            _data[_dataIndex++] = color.A;
        }

        ++_pixel.X;
    }

    if (_pixel.X >= _ihdr.Width) {
        next_line_non_interlaced();
    }
}

void png_decoder::get_image_data_non_interlaced_I8(std::span<u8 const> dat)
{
    _curLine[_pixel.X] = dat[0];
    filter(_pixel.X, _pixel.Y, 1);

    u8 const idx {_curLine[_pixel.X]};
    assert(_plte && _plte->Entries.size() > idx);

    if (_dataIndex + 3 < std::ssize(_data)) {
        auto const color {_plte->Entries[idx]};
        _data[_dataIndex++] = color.R;
        _data[_dataIndex++] = color.G;
        _data[_dataIndex++] = color.B;
        _data[_dataIndex++] = color.A;
    }

    ++_pixel.X;

    if (_pixel.X >= _ihdr.Width) {
        next_line_non_interlaced();
    }
}

void png_decoder::get_image_data_non_interlaced_TC8(std::span<u8 const> dat)
{
    memcpy(_curLine.data() + _lineIndex, dat.data(), dat.size());
    filter(_pixel.X, _pixel.Y, 3);

    if (_dataIndex + 3 < std::ssize(_data)) {
        u8 const r {_curLine[_lineIndex++]};
        u8 const g {_curLine[_lineIndex++]};
        u8 const b {_curLine[_lineIndex++]};

        _data[_dataIndex + 3] = _trns && _trns->is_rgb_transparent(r, g, b) ? 0 : 255;
        _data[_dataIndex + 2] = b;
        _data[_dataIndex + 1] = g;
        _data[_dataIndex]     = r;
    }

    _dataIndex += 4;
    ++_pixel.X;

    if (_pixel.X >= _ihdr.Width) {
        next_line_non_interlaced();
    }
}

void png_decoder::get_image_data_non_interlaced_TC16(std::span<u8 const> dat)
{
    memcpy(_curLine.data() + _lineIndex, dat.data(), dat.size());
    filter(_pixel.X, _pixel.Y, 6);

    if (_dataIndex + 3 < std::ssize(_data)) {
        u8 const r {_curLine[_lineIndex]};
        u8 const g {_curLine[_lineIndex + 2]};
        u8 const b {_curLine[_lineIndex + 4]};

        _data[_dataIndex + 3] = _trns && _trns->is_rgb_transparent(r, g, b) ? 0 : 255;
        _data[_dataIndex + 2] = b;
        _data[_dataIndex + 1] = g;
        _data[_dataIndex]     = r;
    }

    _dataIndex += 4;
    _lineIndex += 6;
    ++_pixel.X;

    if (_pixel.X >= _ihdr.Width) {
        next_line_non_interlaced();
    }
}

void png_decoder::get_image_data_non_interlaced_TCA8(std::span<u8 const> dat)
{
    memcpy(_curLine.data() + _lineIndex, dat.data(), dat.size());
    filter(_pixel.X, _pixel.Y, 4);

    if (_dataIndex + 3 < std::ssize(_data)) {
        for (i32 i {0}; i < 4; ++i) {
            _data[_dataIndex++] = _curLine[_lineIndex++];
        }
    }

    ++_pixel.X;

    if (_pixel.X >= _ihdr.Width) {
        next_line_non_interlaced();
    }
}

void png_decoder::get_image_data_non_interlaced_TCA16(std::span<u8 const> dat)
{
    memcpy(_curLine.data() + _lineIndex, dat.data(), dat.size());
    filter(_pixel.X, _pixel.Y, 8);

    if (_dataIndex + 3 < std::ssize(_data)) {
        for (i32 i {0}; i < 4; ++i) {
            _data[_dataIndex++] = _curLine[_lineIndex];
            _lineIndex += 2;
        }
    }

    ++_pixel.X;

    if (_pixel.X >= _ihdr.Width) {
        next_line_non_interlaced();
    }
}

}
