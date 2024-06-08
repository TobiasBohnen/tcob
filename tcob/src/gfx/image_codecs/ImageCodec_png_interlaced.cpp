// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "ImageCodec_png.hpp"

#include <cstring>

namespace tcob::gfx::detail {

void png_decoder::get_image_data_interlaced_G1(std::span<u8 const> dat)
{
    auto iRect {get_interlace_dimensions()};

    _curLine[_pixel.X / 8] = dat[0];
    filter8(_pixel.X / 8, _pixel.Y, 1);

    for (i32 i {0}; i < 8 && _pixel.X < _ihdr.Width; i++) {
        u8 const c {static_cast<u8>(png::get_bits(_curLine[_pixel.X / 8], 7 - i, 1) * 255)};
        _dataIndex = iRect.X * png::BPP + iRect.Y * _stride;
        if (_dataIndex + 3 < std::ssize(_data)) {
            _data[_dataIndex]     = c;
            _data[_dataIndex + 1] = c;
            _data[_dataIndex + 2] = c;
            _data[_dataIndex + 3] = _trns && _trns->is_gray_transparent(c) ? 0 : 255;
        }

        ++_pixel.X;

        iRect = get_interlace_dimensions();
    }

    if (_pixel.X >= iRect.Width) {
        next_line_interlaced(iRect.Height);
    }
}

void png_decoder::get_image_data_interlaced_G2(std::span<u8 const> dat)
{
    auto iRect {get_interlace_dimensions()};

    _curLine[_pixel.X / 4] = dat[0];
    filter8(_pixel.X / 4, _pixel.Y, 1);

    for (i32 i {0}; i < 8 && _pixel.X < _ihdr.Width; i += 2) {
        u8 const c {static_cast<u8>(png::get_bits(_curLine[_pixel.X / 4], 6 - i, 2) / 3.0f * 255)};
        _dataIndex = iRect.X * png::BPP + iRect.Y * _stride;
        if (_dataIndex + 3 < std::ssize(_data)) {
            _data[_dataIndex]     = c;
            _data[_dataIndex + 1] = c;
            _data[_dataIndex + 2] = c;
            _data[_dataIndex + 3] = _trns && _trns->is_gray_transparent(c) ? 0 : 255;
        }

        ++_pixel.X;

        iRect = get_interlace_dimensions();
    }

    if (_pixel.X >= iRect.Width) {
        next_line_interlaced(iRect.Height);
    }
}

void png_decoder::get_image_data_interlaced_G4(std::span<u8 const> dat)
{
    auto iRect {get_interlace_dimensions()};

    _curLine[_pixel.X / 2] = dat[0];
    filter8(_pixel.X / 2, _pixel.Y, 1);

    for (i32 i {0}; i < 8 && _pixel.X < _ihdr.Width; i += 4) {
        u8 const c {static_cast<u8>(png::get_bits(_curLine[_pixel.X / 2], 4 - i, 4) / 15.0f * 255)};
        _dataIndex = iRect.X * png::BPP + iRect.Y * _stride;
        if (_dataIndex + 3 < std::ssize(_data)) {
            _data[_dataIndex]     = c;
            _data[_dataIndex + 1] = c;
            _data[_dataIndex + 2] = c;
            _data[_dataIndex + 3] = _trns && _trns->is_gray_transparent(c) ? 0 : 255;
        }

        ++_pixel.X;

        iRect = get_interlace_dimensions();
    }

    if (_pixel.X >= iRect.Width) {
        next_line_interlaced(iRect.Height);
    }
}

void png_decoder::get_image_data_interlaced_G8(std::span<u8 const> dat)
{
    auto const [ix, iy, iw, ih] {get_interlace_dimensions()};

    _curLine[_inLineCount] = dat[0];
    filter8(_pixel.X, _pixel.Y, 1);

    u8 const c {_curLine[_inLineCount]};
    _dataIndex = ix * png::BPP + iy * _stride;
    if (_dataIndex + 3 < std::ssize(_data)) {
        _data[_dataIndex]     = c;
        _data[_dataIndex + 1] = c;
        _data[_dataIndex + 2] = c;
        _data[_dataIndex + 3] = _trns && _trns->is_gray_transparent(c) ? 0 : 255;
    }

    ++_pixel.X;
    _inLineCount++;

    if (_pixel.X >= iw) {
        next_line_interlaced(ih);
    }
}

void png_decoder::get_image_data_interlaced_G16(std::span<u8 const> dat)
{
    auto const [ix, iy, iw, ih] {get_interlace_dimensions()};

    memcpy(_curLine.data() + _inLineCount, dat.data(), dat.size());
    filter8(_pixel.X, _pixel.Y, 2);

    u8 const c {_curLine[_inLineCount]};
    _dataIndex = ix * png::BPP + iy * _stride;
    if (_dataIndex + 3 < std::ssize(_data)) {
        _data[_dataIndex]     = c;
        _data[_dataIndex + 1] = c;
        _data[_dataIndex + 2] = c;
        _data[_dataIndex + 3] = _trns && _trns->is_gray_transparent(c) ? 0 : 255;
    }

    ++_pixel.X;
    _inLineCount += 2;

    if (_pixel.X >= iw) {
        next_line_interlaced(ih);
    }
}

void png_decoder::get_image_data_interlaced_GA8(std::span<u8 const> dat)
{
    auto const [ix, iy, iw, ih] {get_interlace_dimensions()};

    memcpy(_curLine.data() + _inLineCount, dat.data(), dat.size());
    filter8(_pixel.X, _pixel.Y, 2);

    _dataIndex = ix * png::BPP + iy * _stride;
    if (_dataIndex + 3 < std::ssize(_data)) {
        _data[_dataIndex] = _data[_dataIndex + 1] = _data[_dataIndex + 2] = _curLine[_inLineCount];
        _data[_dataIndex + 3]                                             = _curLine[_inLineCount + 1];
    }

    ++_pixel.X;
    _inLineCount += 2;

    if (_pixel.X >= iw) {
        next_line_interlaced(ih);
    }
}

void png_decoder::get_image_data_interlaced_GA16(std::span<u8 const> dat)
{
    auto const [ix, iy, iw, ih] {get_interlace_dimensions()};

    memcpy(_curLine.data() + _inLineCount, dat.data(), dat.size());
    filter8(_pixel.X, _pixel.Y, 4);

    _dataIndex = ix * png::BPP + iy * _stride;
    if (_dataIndex + 3 < std::ssize(_data)) {
        _data[_dataIndex] = _data[_dataIndex + 1] = _data[_dataIndex + 2] = _curLine[_inLineCount];
        _data[_dataIndex + 3]                                             = _curLine[_inLineCount + 2];
    }

    ++_pixel.X;
    _inLineCount += 4;

    if (_pixel.X >= iw) {
        next_line_interlaced(ih);
    }
}

void png_decoder::get_image_data_interlaced_I1(std::span<u8 const> dat)
{
    auto iRect {get_interlace_dimensions()};

    _curLine[_pixel.X / 8] = dat[0];
    filter8(_pixel.X / 8, _pixel.Y, 1);

    for (i32 i {0}; i < 8 && _pixel.X < _ihdr.Width; i++) {
        _dataIndex = iRect.X * png::BPP + iRect.Y * _stride;
        if (_dataIndex + 3 < std::ssize(_data)) {
            u8 const idx {png::get_bits(_curLine[_pixel.X / 8], 7 - i, 1)};
            assert(_plte && _plte->Entries.size() > idx);

            auto const color {_plte->Entries[idx]};
            _data[_dataIndex]     = color.R;
            _data[_dataIndex + 1] = color.G;
            _data[_dataIndex + 2] = color.B;
            _data[_dataIndex + 3] = color.A;
        }

        ++_pixel.X;

        iRect = get_interlace_dimensions();
    }

    if (_pixel.X >= iRect.Width) {
        next_line_interlaced(iRect.Height);
    }
}

void png_decoder::get_image_data_interlaced_I2(std::span<u8 const> dat)
{
    auto iRect {get_interlace_dimensions()};

    _curLine[_pixel.X / 4] = dat[0];
    filter8(_pixel.X / 4, _pixel.Y, 1);

    for (i32 i {0}; i < 8 && _pixel.X < _ihdr.Width; i += 2) {
        _dataIndex = iRect.X * png::BPP + iRect.Y * _stride;
        if (_dataIndex + 3 < std::ssize(_data)) {
            u8 const idx {png::get_bits(_curLine[_pixel.X / 4], 6 - i, 2)};
            assert(_plte && _plte->Entries.size() > idx);

            auto const color {_plte->Entries[idx]};
            _data[_dataIndex]     = color.R;
            _data[_dataIndex + 1] = color.G;
            _data[_dataIndex + 2] = color.B;
            _data[_dataIndex + 3] = color.A;
        }

        ++_pixel.X;

        iRect = get_interlace_dimensions();
    }

    if (_pixel.X >= iRect.Width) {
        next_line_interlaced(iRect.Height);
    }
}

void png_decoder::get_image_data_interlaced_I4(std::span<u8 const> dat)
{
    auto iRect {get_interlace_dimensions()};

    _curLine[_pixel.X / 2] = dat[0];
    filter8(_pixel.X / 2, _pixel.Y, 1);

    for (i32 i {0}; i < 8 && _pixel.X < _ihdr.Width; i += 4) {
        _dataIndex = iRect.X * png::BPP + iRect.Y * _stride;
        if (_dataIndex + 3 < std::ssize(_data)) {
            u8 const idx {png::get_bits(_curLine[_pixel.X / 2], 4 - i, 4)};
            assert(_plte && _plte->Entries.size() > idx);

            auto const color {_plte->Entries[idx]};
            _data[_dataIndex]     = color.R;
            _data[_dataIndex + 1] = color.G;
            _data[_dataIndex + 2] = color.B;
            _data[_dataIndex + 3] = color.A;
        }

        ++_pixel.X;

        iRect = get_interlace_dimensions();
    }

    if (_pixel.X >= iRect.Width) {
        next_line_interlaced(iRect.Height);
    }
}

void png_decoder::get_image_data_interlaced_I8(std::span<u8 const> dat)
{
    auto const [ix, iy, iw, ih] {get_interlace_dimensions()};

    _curLine[_inLineCount] = dat[0];
    filter8(_pixel.X, _pixel.Y, 1);

    _dataIndex = ix * png::BPP + iy * _stride;
    if (_dataIndex + 3 < std::ssize(_data)) {
        u8 const idx {_curLine[_inLineCount]};
        assert(_plte && _plte->Entries.size() > idx);

        auto const color {_plte->Entries[idx]};
        _data[_dataIndex]     = color.R;
        _data[_dataIndex + 1] = color.G;
        _data[_dataIndex + 2] = color.B;
        _data[_dataIndex + 3] = color.A;
    }

    ++_pixel.X;
    _inLineCount++;

    if (_pixel.X >= iw) {
        next_line_interlaced(ih);
    }
}

void png_decoder::get_image_data_interlaced_TC8(std::span<u8 const> dat)
{
    auto const [ix, iy, iw, ih] {get_interlace_dimensions()};

    memcpy(_curLine.data() + _inLineCount, dat.data(), dat.size());
    filter8(_pixel.X, _pixel.Y, 3);

    _dataIndex = ix * png::BPP + iy * _stride;
    if (_dataIndex + 3 < std::ssize(_data)) {
        u8 const r {_curLine[_inLineCount]};
        u8 const g {_curLine[_inLineCount + 1]};
        u8 const b {_curLine[_inLineCount + 2]};
        _data[_dataIndex + 3] = _trns && _trns->is_rgb_transparent(r, g, b) ? 0 : 255;
        _data[_dataIndex + 2] = b;
        _data[_dataIndex + 1] = g;
        _data[_dataIndex]     = r;
    }

    ++_pixel.X;
    _inLineCount += 3;

    if (_pixel.X >= iw) {
        next_line_interlaced(ih);
    }
}

void png_decoder::get_image_data_interlaced_TC16(std::span<u8 const> dat)
{
    auto const [ix, iy, iw, ih] {get_interlace_dimensions()};

    memcpy(_curLine.data() + _inLineCount, dat.data(), dat.size());
    filter8(_pixel.X, _pixel.Y, 6);

    _dataIndex = ix * png::BPP + iy * _stride;
    if (_dataIndex + 3 < std::ssize(_data)) {
        u8 const r {_curLine[_inLineCount]};
        u8 const g {_curLine[_inLineCount + 2]};
        u8 const b {_curLine[_inLineCount + 4]};
        _data[_dataIndex + 3] = _trns && _trns->is_rgb_transparent(r, g, b) ? 0 : 255;
        _data[_dataIndex + 2] = b;
        _data[_dataIndex + 1] = g;
        _data[_dataIndex]     = r;
    }

    ++_pixel.X;
    _inLineCount += 6;

    if (_pixel.X >= iw) {
        next_line_interlaced(ih);
    }
}

void png_decoder::get_image_data_interlaced_TCA8(std::span<u8 const> dat)
{
    auto const [ix, iy, iw, ih] {get_interlace_dimensions()};

    memcpy(_curLine.data() + _inLineCount, dat.data(), dat.size());
    filter8(_pixel.X, _pixel.Y, 4);

    _dataIndex = ix * png::BPP + iy * _stride;
    if (_dataIndex + 3 < std::ssize(_data)) {
        _data[_dataIndex]     = _curLine[_inLineCount];
        _data[_dataIndex + 1] = _curLine[_inLineCount + 1];
        _data[_dataIndex + 2] = _curLine[_inLineCount + 2];
        _data[_dataIndex + 3] = _curLine[_inLineCount + 3];
    }

    ++_pixel.X;
    _inLineCount += 4;

    if (_pixel.X >= iw) {
        next_line_interlaced(ih);
    }
}

void png_decoder::get_image_data_interlaced_TCA16(std::span<u8 const> dat)
{
    auto const [ix, iy, iw, ih] {get_interlace_dimensions()};

    memcpy(_curLine.data() + _inLineCount, dat.data(), dat.size());
    filter8(_pixel.X, _pixel.Y, 8);

    _dataIndex = ix * png::BPP + iy * _stride;
    if (_dataIndex + 3 < std::ssize(_data)) {
        _data[_dataIndex]     = _curLine[_inLineCount];
        _data[_dataIndex + 1] = _curLine[_inLineCount + 2];
        _data[_dataIndex + 2] = _curLine[_inLineCount + 4];
        _data[_dataIndex + 3] = _curLine[_inLineCount + 6];
    }

    ++_pixel.X;
    _inLineCount += 8;

    if (_pixel.X >= iw) {
        next_line_interlaced(ih);
    }
}

}
