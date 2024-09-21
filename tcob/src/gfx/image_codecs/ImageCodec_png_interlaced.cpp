// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "ImageCodec_png.hpp"

namespace tcob::gfx::detail {

void png_decoder::interlaced_G1()
{
    auto iRect {get_interlace_dimensions()};

    for (i32 i {0}; i < 8 && _pixel.X < _ihdr.Width; i++) {
        u8 const  c {static_cast<u8>(helper::get_bits(*_curLineIt, 7 - i, 1) * 255)};
        i32 const dataIndex {(iRect.X * png::BPP) + (iRect.Y * _ihdr.Width * png::BPP)};
        if (dataIndex + 3 < std::ssize(_data)) {
            _data[dataIndex]     = c;
            _data[dataIndex + 1] = c;
            _data[dataIndex + 2] = c;
            _data[dataIndex + 3] = _trns && _trns->is_gray_transparent(c) ? 0 : 255;
        }

        ++_pixel.X;

        iRect = get_interlace_dimensions();
    }

    if (_pixel.X >= iRect.Width) { next_line_interlaced(iRect.Height); }
}

void png_decoder::interlaced_G2()
{
    auto iRect {get_interlace_dimensions()};

    for (i32 i {0}; i < 8 && _pixel.X < _ihdr.Width; i += 2) {
        u8 const  c {static_cast<u8>(helper::get_bits(*_curLineIt, 6 - i, 2) / 3.0f * 255)};
        i32 const dataIndex {(iRect.X * png::BPP) + (iRect.Y * _ihdr.Width * png::BPP)};
        if (dataIndex + 3 < std::ssize(_data)) {
            _data[dataIndex]     = c;
            _data[dataIndex + 1] = c;
            _data[dataIndex + 2] = c;
            _data[dataIndex + 3] = _trns && _trns->is_gray_transparent(c) ? 0 : 255;
        }

        ++_pixel.X;

        iRect = get_interlace_dimensions();
    }

    if (_pixel.X >= iRect.Width) { next_line_interlaced(iRect.Height); }
}

void png_decoder::interlaced_G4()
{
    auto iRect {get_interlace_dimensions()};

    for (i32 i {0}; i < 8 && _pixel.X < _ihdr.Width; i += 4) {
        u8 const  c {static_cast<u8>(helper::get_bits(*_curLineIt, 4 - i, 4) / 15.0f * 255)};
        i32 const dataIndex {(iRect.X * png::BPP) + (iRect.Y * _ihdr.Width * png::BPP)};
        if (dataIndex + 3 < std::ssize(_data)) {
            _data[dataIndex]     = c;
            _data[dataIndex + 1] = c;
            _data[dataIndex + 2] = c;
            _data[dataIndex + 3] = _trns && _trns->is_gray_transparent(c) ? 0 : 255;
        }

        ++_pixel.X;

        iRect = get_interlace_dimensions();
    }

    if (_pixel.X >= iRect.Width) { next_line_interlaced(iRect.Height); }
}

void png_decoder::interlaced_G8_16()
{
    auto const [ix, iy, iw, ih] {get_interlace_dimensions()};

    u8 const  c {*_curLineIt};
    i32 const dataIndex {(ix * png::BPP) + (iy * _ihdr.Width * png::BPP)};
    if (dataIndex + 3 < std::ssize(_data)) {
        _data[dataIndex]     = c;
        _data[dataIndex + 1] = c;
        _data[dataIndex + 2] = c;
        _data[dataIndex + 3] = _trns && _trns->is_gray_transparent(c) ? 0 : 255;
    }

    if (++_pixel.X >= iw) { next_line_interlaced(ih); }
}

void png_decoder::interlaced_GA8_16()
{
    auto const [ix, iy, iw, ih] {get_interlace_dimensions()};

    i32 const dataIndex {(ix * png::BPP) + (iy * _ihdr.Width * png::BPP)};
    if (dataIndex + 3 < std::ssize(_data)) {
        _data[dataIndex] = _data[dataIndex + 1] = _data[dataIndex + 2] = *_curLineIt;
        _data[dataIndex + 3]                                           = *(_curLineIt + (_pixelSize / 2));
    }

    if (++_pixel.X >= iw) { next_line_interlaced(ih); }
}

void png_decoder::interlaced_I1()
{
    auto iRect {get_interlace_dimensions()};

    for (i32 i {0}; i < 8 && _pixel.X < _ihdr.Width; i++) {
        i32 const dataIndex {(iRect.X * png::BPP) + (iRect.Y * _ihdr.Width * png::BPP)};
        if (dataIndex + 3 < std::ssize(_data)) {
            u8 const idx {static_cast<u8>(helper::get_bits(*_curLineIt, 7 - i, 1))};

            auto const color {_plte->Entries.at(idx)};
            _data[dataIndex]     = color.R;
            _data[dataIndex + 1] = color.G;
            _data[dataIndex + 2] = color.B;
            _data[dataIndex + 3] = color.A;
        }

        ++_pixel.X;

        iRect = get_interlace_dimensions();
    }

    if (_pixel.X >= iRect.Width) { next_line_interlaced(iRect.Height); }
}

void png_decoder::interlaced_I2()
{
    auto iRect {get_interlace_dimensions()};

    for (i32 i {0}; i < 8 && _pixel.X < _ihdr.Width; i += 2) {
        i32 const dataIndex {(iRect.X * png::BPP) + (iRect.Y * _ihdr.Width * png::BPP)};
        if (dataIndex + 3 < std::ssize(_data)) {
            u8 const idx {static_cast<u8>(helper::get_bits(*_curLineIt, 6 - i, 2))};

            auto const color {_plte->Entries.at(idx)};
            _data[dataIndex]     = color.R;
            _data[dataIndex + 1] = color.G;
            _data[dataIndex + 2] = color.B;
            _data[dataIndex + 3] = color.A;
        }

        ++_pixel.X;

        iRect = get_interlace_dimensions();
    }

    if (_pixel.X >= iRect.Width) { next_line_interlaced(iRect.Height); }
}

void png_decoder::interlaced_I4()
{
    auto iRect {get_interlace_dimensions()};

    for (i32 i {0}; i < 8 && _pixel.X < _ihdr.Width; i += 4) {
        i32 const dataIndex {(iRect.X * png::BPP) + (iRect.Y * _ihdr.Width * png::BPP)};
        if (dataIndex + 3 < std::ssize(_data)) {
            u8 const idx {static_cast<u8>(helper::get_bits(*_curLineIt, 4 - i, 4))};

            auto const color {_plte->Entries.at(idx)};
            _data[dataIndex]     = color.R;
            _data[dataIndex + 1] = color.G;
            _data[dataIndex + 2] = color.B;
            _data[dataIndex + 3] = color.A;
        }

        ++_pixel.X;

        iRect = get_interlace_dimensions();
    }

    if (_pixel.X >= iRect.Width) { next_line_interlaced(iRect.Height); }
}

void png_decoder::interlaced_I8()
{
    auto const [ix, iy, iw, ih] {get_interlace_dimensions()};

    i32 const dataIndex {(ix * png::BPP) + (iy * _ihdr.Width * png::BPP)};
    if (dataIndex + 3 < std::ssize(_data)) {
        u8 const idx {*_curLineIt};

        auto const color {_plte->Entries.at(idx)};
        _data[dataIndex]     = color.R;
        _data[dataIndex + 1] = color.G;
        _data[dataIndex + 2] = color.B;
        _data[dataIndex + 3] = color.A;
    }

    if (++_pixel.X >= iw) { next_line_interlaced(ih); }
}

void png_decoder::interlaced_TC8_16()
{
    auto const [ix, iy, iw, ih] {get_interlace_dimensions()};

    i32 const dataIndex {(ix * png::BPP) + (iy * _ihdr.Width * png::BPP)};
    if (dataIndex + 3 < std::ssize(_data)) {
        u8 const r {*_curLineIt};
        u8 const g {*(_curLineIt + (_pixelSize / 3))};
        u8 const b {*(_curLineIt + (_pixelSize / 3 * 2))};
        u8 const a {_trns && _trns->is_rgb_transparent(r, g, b) ? u8 {0} : u8 {255}};
        _data[dataIndex]     = r;
        _data[dataIndex + 1] = g;
        _data[dataIndex + 2] = b;
        _data[dataIndex + 3] = a;
    }

    if (++_pixel.X >= iw) { next_line_interlaced(ih); }
}

void png_decoder::interlaced_TCA8_16()
{
    auto const [ix, iy, iw, ih] {get_interlace_dimensions()};

    i32 dataIndex {(ix * png::BPP) + (iy * _ihdr.Width * png::BPP)};
    if (dataIndex + 3 < std::ssize(_data)) {
        for (i32 i {0}; i < 4; ++i) {
            _data[dataIndex++] = *(_curLineIt + (i * _pixelSize / 4));
        }
    }

    if (++_pixel.X >= iw) { next_line_interlaced(ih); }
}

}
