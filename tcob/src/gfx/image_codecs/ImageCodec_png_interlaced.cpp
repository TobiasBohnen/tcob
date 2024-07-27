// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "ImageCodec_png.hpp"

#include <cstring>

namespace tcob::gfx::detail {

void png_decoder::get_image_data_interlaced_G1(std::span<u8 const> pix)
{
    auto iRect {get_interlace_dimensions()};

    _curLine[_pixel.X / 8] = pix[0];
    filter(_pixel.X / 8, _pixel.Y, 1);

    for (i32 i {0}; i < 8 && _pixel.X < _ihdr.Width; i++) {
        u8 const  c {static_cast<u8>(png::get_bits(_curLine[_pixel.X / 8], 7 - i, 1) * 255)};
        i32 const dataIndex {iRect.X * png::BPP + iRect.Y * _ihdr.Width * png::BPP};
        if (dataIndex + 3 < std::ssize(_data)) {
            _data[dataIndex]     = c;
            _data[dataIndex + 1] = c;
            _data[dataIndex + 2] = c;
            _data[dataIndex + 3] = _trns && _trns->is_gray_transparent(c) ? 0 : 255;
        }

        ++_pixel.X;

        iRect = get_interlace_dimensions();
    }

    if (_pixel.X >= iRect.Width) {
        next_line_interlaced(iRect.Height);
    }
}

void png_decoder::get_image_data_interlaced_G2(std::span<u8 const> pix)
{
    auto iRect {get_interlace_dimensions()};

    _curLine[_pixel.X / 4] = pix[0];
    filter(_pixel.X / 4, _pixel.Y, 1);

    for (i32 i {0}; i < 8 && _pixel.X < _ihdr.Width; i += 2) {
        u8 const  c {static_cast<u8>(png::get_bits(_curLine[_pixel.X / 4], 6 - i, 2) / 3.0f * 255)};
        i32 const dataIndex {iRect.X * png::BPP + iRect.Y * _ihdr.Width * png::BPP};
        if (dataIndex + 3 < std::ssize(_data)) {
            _data[dataIndex]     = c;
            _data[dataIndex + 1] = c;
            _data[dataIndex + 2] = c;
            _data[dataIndex + 3] = _trns && _trns->is_gray_transparent(c) ? 0 : 255;
        }

        ++_pixel.X;

        iRect = get_interlace_dimensions();
    }

    if (_pixel.X >= iRect.Width) {
        next_line_interlaced(iRect.Height);
    }
}

void png_decoder::get_image_data_interlaced_G4(std::span<u8 const> pix)
{
    auto iRect {get_interlace_dimensions()};

    _curLine[_pixel.X / 2] = pix[0];
    filter(_pixel.X / 2, _pixel.Y, 1);

    for (i32 i {0}; i < 8 && _pixel.X < _ihdr.Width; i += 4) {
        u8 const  c {static_cast<u8>(png::get_bits(_curLine[_pixel.X / 2], 4 - i, 4) / 15.0f * 255)};
        i32 const dataIndex {iRect.X * png::BPP + iRect.Y * _ihdr.Width * png::BPP};
        if (dataIndex + 3 < std::ssize(_data)) {
            _data[dataIndex]     = c;
            _data[dataIndex + 1] = c;
            _data[dataIndex + 2] = c;
            _data[dataIndex + 3] = _trns && _trns->is_gray_transparent(c) ? 0 : 255;
        }

        ++_pixel.X;

        iRect = get_interlace_dimensions();
    }

    if (_pixel.X >= iRect.Width) {
        next_line_interlaced(iRect.Height);
    }
}

void png_decoder::get_image_data_interlaced_G8(std::span<u8 const> pix)
{
    auto const [ix, iy, iw, ih] {get_interlace_dimensions()};

    _curLine[_lineIndex] = pix[0];
    filter(_pixel.X, _pixel.Y, 1);

    u8 const  c {_curLine[_lineIndex++]};
    i32 const dataIndex {ix * png::BPP + iy * _ihdr.Width * png::BPP};
    if (dataIndex + 3 < std::ssize(_data)) {
        _data[dataIndex]     = c;
        _data[dataIndex + 1] = c;
        _data[dataIndex + 2] = c;
        _data[dataIndex + 3] = _trns && _trns->is_gray_transparent(c) ? 0 : 255;
    }

    ++_pixel.X;

    if (_pixel.X >= iw) {
        next_line_interlaced(ih);
    }
}

void png_decoder::get_image_data_interlaced_G16(std::span<u8 const> pix)
{
    auto const [ix, iy, iw, ih] {get_interlace_dimensions()};

    memcpy(_curLine.data() + _lineIndex, pix.data(), pix.size());
    filter(_pixel.X, _pixel.Y, 2);

    u8 const  c {_curLine[_lineIndex]};
    i32 const dataIndex {ix * png::BPP + iy * _ihdr.Width * png::BPP};
    if (dataIndex + 3 < std::ssize(_data)) {
        _data[dataIndex]     = c;
        _data[dataIndex + 1] = c;
        _data[dataIndex + 2] = c;
        _data[dataIndex + 3] = _trns && _trns->is_gray_transparent(c) ? 0 : 255;
    }

    _lineIndex += 2;
    ++_pixel.X;

    if (_pixel.X >= iw) {
        next_line_interlaced(ih);
    }
}

void png_decoder::get_image_data_interlaced_GA8(std::span<u8 const> pix)
{
    auto const [ix, iy, iw, ih] {get_interlace_dimensions()};

    memcpy(_curLine.data() + _lineIndex, pix.data(), pix.size());
    filter(_pixel.X, _pixel.Y, 2);

    i32 const dataIndex {ix * png::BPP + iy * _ihdr.Width * png::BPP};
    if (dataIndex + 3 < std::ssize(_data)) {
        _data[dataIndex] = _data[dataIndex + 1] = _data[dataIndex + 2] = _curLine[_lineIndex++];
        _data[dataIndex + 3]                                           = _curLine[_lineIndex++];
    }

    ++_pixel.X;

    if (_pixel.X >= iw) {
        next_line_interlaced(ih);
    }
}

void png_decoder::get_image_data_interlaced_GA16(std::span<u8 const> pix)
{
    auto const [ix, iy, iw, ih] {get_interlace_dimensions()};

    memcpy(_curLine.data() + _lineIndex, pix.data(), pix.size());
    filter(_pixel.X, _pixel.Y, 4);

    i32 const dataIndex {ix * png::BPP + iy * _ihdr.Width * png::BPP};
    if (dataIndex + 3 < std::ssize(_data)) {
        _data[dataIndex] = _data[dataIndex + 1] = _data[dataIndex + 2] = _curLine[_lineIndex];
        _data[dataIndex + 3]                                           = _curLine[_lineIndex + 2];
    }

    _lineIndex += 4;
    ++_pixel.X;

    if (_pixel.X >= iw) {
        next_line_interlaced(ih);
    }
}

void png_decoder::get_image_data_interlaced_I1(std::span<u8 const> pix)
{
    auto iRect {get_interlace_dimensions()};

    _curLine[_pixel.X / 8] = pix[0];
    filter(_pixel.X / 8, _pixel.Y, 1);

    for (i32 i {0}; i < 8 && _pixel.X < _ihdr.Width; i++) {
        i32 const dataIndex {iRect.X * png::BPP + iRect.Y * _ihdr.Width * png::BPP};
        if (dataIndex + 3 < std::ssize(_data)) {
            u8 const idx {png::get_bits(_curLine[_pixel.X / 8], 7 - i, 1)};
            assert(_plte && _plte->Entries.size() > idx);

            auto const color {_plte->Entries[idx]};
            _data[dataIndex]     = color.R;
            _data[dataIndex + 1] = color.G;
            _data[dataIndex + 2] = color.B;
            _data[dataIndex + 3] = color.A;
        }

        ++_pixel.X;

        iRect = get_interlace_dimensions();
    }

    if (_pixel.X >= iRect.Width) {
        next_line_interlaced(iRect.Height);
    }
}

void png_decoder::get_image_data_interlaced_I2(std::span<u8 const> pix)
{
    auto iRect {get_interlace_dimensions()};

    _curLine[_pixel.X / 4] = pix[0];
    filter(_pixel.X / 4, _pixel.Y, 1);

    for (i32 i {0}; i < 8 && _pixel.X < _ihdr.Width; i += 2) {
        i32 const dataIndex {iRect.X * png::BPP + iRect.Y * _ihdr.Width * png::BPP};
        if (dataIndex + 3 < std::ssize(_data)) {
            u8 const idx {png::get_bits(_curLine[_pixel.X / 4], 6 - i, 2)};
            assert(_plte && _plte->Entries.size() > idx);

            auto const color {_plte->Entries[idx]};
            _data[dataIndex]     = color.R;
            _data[dataIndex + 1] = color.G;
            _data[dataIndex + 2] = color.B;
            _data[dataIndex + 3] = color.A;
        }

        ++_pixel.X;

        iRect = get_interlace_dimensions();
    }

    if (_pixel.X >= iRect.Width) {
        next_line_interlaced(iRect.Height);
    }
}

void png_decoder::get_image_data_interlaced_I4(std::span<u8 const> pix)
{
    auto iRect {get_interlace_dimensions()};

    _curLine[_pixel.X / 2] = pix[0];
    filter(_pixel.X / 2, _pixel.Y, 1);

    for (i32 i {0}; i < 8 && _pixel.X < _ihdr.Width; i += 4) {
        i32 const dataIndex {iRect.X * png::BPP + iRect.Y * _ihdr.Width * png::BPP};
        if (dataIndex + 3 < std::ssize(_data)) {
            u8 const idx {png::get_bits(_curLine[_pixel.X / 2], 4 - i, 4)};
            assert(_plte && _plte->Entries.size() > idx);

            auto const color {_plte->Entries[idx]};
            _data[dataIndex]     = color.R;
            _data[dataIndex + 1] = color.G;
            _data[dataIndex + 2] = color.B;
            _data[dataIndex + 3] = color.A;
        }

        ++_pixel.X;

        iRect = get_interlace_dimensions();
    }

    if (_pixel.X >= iRect.Width) {
        next_line_interlaced(iRect.Height);
    }
}

void png_decoder::get_image_data_interlaced_I8(std::span<u8 const> pix)
{
    auto const [ix, iy, iw, ih] {get_interlace_dimensions()};

    _curLine[_lineIndex] = pix[0];
    filter(_pixel.X, _pixel.Y, 1);

    i32 const dataIndex {ix * png::BPP + iy * _ihdr.Width * png::BPP};
    if (dataIndex + 3 < std::ssize(_data)) {
        u8 const idx {_curLine[_lineIndex++]};
        assert(_plte && _plte->Entries.size() > idx);

        auto const color {_plte->Entries[idx]};
        _data[dataIndex]     = color.R;
        _data[dataIndex + 1] = color.G;
        _data[dataIndex + 2] = color.B;
        _data[dataIndex + 3] = color.A;
    }

    ++_pixel.X;

    if (_pixel.X >= iw) {
        next_line_interlaced(ih);
    }
}

void png_decoder::get_image_data_interlaced_TC8(std::span<u8 const> pix)
{
    auto const [ix, iy, iw, ih] {get_interlace_dimensions()};

    memcpy(_curLine.data() + _lineIndex, pix.data(), pix.size());
    filter(_pixel.X, _pixel.Y, 3);

    i32 const dataIndex {ix * png::BPP + iy * _ihdr.Width * png::BPP};
    if (dataIndex + 3 < std::ssize(_data)) {
        u8 const r {_curLine[_lineIndex++]};
        u8 const g {_curLine[_lineIndex++]};
        u8 const b {_curLine[_lineIndex++]};
        _data[dataIndex + 3] = _trns && _trns->is_rgb_transparent(r, g, b) ? 0 : 255;
        _data[dataIndex + 2] = b;
        _data[dataIndex + 1] = g;
        _data[dataIndex]     = r;
    }

    ++_pixel.X;

    if (_pixel.X >= iw) {
        next_line_interlaced(ih);
    }
}

void png_decoder::get_image_data_interlaced_TC16(std::span<u8 const> pix)
{
    auto const [ix, iy, iw, ih] {get_interlace_dimensions()};

    memcpy(_curLine.data() + _lineIndex, pix.data(), pix.size());
    filter(_pixel.X, _pixel.Y, 6);

    i32 const dataIndex {ix * png::BPP + iy * _ihdr.Width * png::BPP};
    if (dataIndex + 3 < std::ssize(_data)) {
        u8 const r {_curLine[_lineIndex]};
        u8 const g {_curLine[_lineIndex + 2]};
        u8 const b {_curLine[_lineIndex + 4]};
        _data[dataIndex + 3] = _trns && _trns->is_rgb_transparent(r, g, b) ? 0 : 255;
        _data[dataIndex + 2] = b;
        _data[dataIndex + 1] = g;
        _data[dataIndex]     = r;
    }

    _lineIndex += 6;
    ++_pixel.X;

    if (_pixel.X >= iw) {
        next_line_interlaced(ih);
    }
}

void png_decoder::get_image_data_interlaced_TCA8(std::span<u8 const> pix)
{
    auto const [ix, iy, iw, ih] {get_interlace_dimensions()};

    memcpy(_curLine.data() + _lineIndex, pix.data(), pix.size());
    filter(_pixel.X, _pixel.Y, 4);

    i32 dataIndex {ix * png::BPP + iy * _ihdr.Width * png::BPP};
    if (dataIndex + 3 < std::ssize(_data)) {
        for (i32 i {0}; i < 4; ++i) {
            _data[dataIndex++] = _curLine[_lineIndex++];
        }
    }

    ++_pixel.X;

    if (_pixel.X >= iw) {
        next_line_interlaced(ih);
    }
}

void png_decoder::get_image_data_interlaced_TCA16(std::span<u8 const> pix)
{
    auto const [ix, iy, iw, ih] {get_interlace_dimensions()};

    memcpy(_curLine.data() + _lineIndex, pix.data(), pix.size());
    filter(_pixel.X, _pixel.Y, 8);

    i32 dataIndex {ix * png::BPP + iy * _ihdr.Width * png::BPP};
    if (dataIndex + 3 < std::ssize(_data)) {
        for (i32 i {0}; i < 4; ++i) {
            _data[dataIndex++] = _curLine[_lineIndex];
            _lineIndex += 2;
        }
    }

    ++_pixel.X;

    if (_pixel.X >= iw) {
        next_line_interlaced(ih);
    }
}

}
